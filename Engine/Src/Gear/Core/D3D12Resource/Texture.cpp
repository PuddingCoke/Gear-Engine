#include<Gear/Core/D3D12Resource/Texture.h>

#include<Gear/Utils/Math.h>

namespace Gear::Core::D3D12Resource
{
	Texture::Texture(const uint32_t width, const uint32_t height, const DXGI_FORMAT format, const uint32_t arraySize, const uint32_t mipLevels, const bool stateTracking, const D3D12_RESOURCE_FLAGS resFlags, const D3D12_CLEAR_VALUE* const clearValue) :
		D3D12ResourceBase(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, CD3DX12_RESOURCE_DESC::Tex2D(format, width, height, arraySize, mipLevels, 1, 0, resFlags), stateTracking, D3D12_RESOURCE_STATE_COPY_DEST, clearValue),
		width(width),
		height(height),
		arraySize(arraySize),
		mipLevels(mipLevels),
		format(format),
		globalState(makeShared<States>(D3D12_RESOURCE_STATE_COPY_DEST, mipLevels)),
		internalState(makeUnique<States>(D3D12_RESOURCE_STATE_COPY_DEST, mipLevels)),
		transitionState(makeUnique<States>(D3D12_RESOURCE_STATE_UNKNOWN, mipLevels)),
		pendingState(makeUnique<States>(D3D12_RESOURCE_STATE_UNKNOWN, mipLevels))
	{

	}

	Texture::Texture(const ComPtr<ID3D12Resource>& texture, const bool stateTracking, const uint32_t initialState) :
		D3D12ResourceBase(texture, stateTracking)
	{
		D3D12_RESOURCE_DESC desc = getResource()->GetDesc();
		width = static_cast<uint32_t>(desc.Width);
		height = desc.Height;
		arraySize = desc.DepthOrArraySize;
		mipLevels = desc.MipLevels;
		format = desc.Format;

		globalState = makeShared<States>(initialState, mipLevels);
		internalState = makeUnique<States>(initialState, mipLevels);
		transitionState = makeUnique<States>(D3D12_RESOURCE_STATE_UNKNOWN, mipLevels);
		pendingState = makeUnique<States>(D3D12_RESOURCE_STATE_UNKNOWN, mipLevels);
	}

	Texture::Texture(Texture& tex) :
		D3D12ResourceBase(tex),
		width(tex.width),
		height(tex.height),
		arraySize(tex.arraySize),
		mipLevels(tex.mipLevels),
		format(tex.format),
		globalState(tex.globalState),
		internalState(makeUnique<States>(D3D12_RESOURCE_STATE_UNKNOWN, mipLevels)),
		transitionState(makeUnique<States>(D3D12_RESOURCE_STATE_UNKNOWN, mipLevels)),
		pendingState(makeUnique<States>(D3D12_RESOURCE_STATE_UNKNOWN, mipLevels))
	{
		tex.resetInternalState();
	}

	Texture::~Texture()
	{
	}

	void Texture::updateGlobalStates()
	{
		if (internalState->allState != D3D12_RESOURCE_STATE_UNKNOWN)
		{
			globalState->set(internalState->allState);
		}
		else
		{
			for (uint32_t mipSlice = 0; mipSlice < mipLevels; mipSlice++)
			{
				if (internalState->mipLevelStates[mipSlice] != D3D12_RESOURCE_STATE_UNKNOWN)
				{
					globalState->mipLevelStates[mipSlice] = internalState->mipLevelStates[mipSlice];
				}
			}

			const uint32_t tempState = globalState->mipLevelStates[0];

			const bool uniformState = globalState->allOfEqual(tempState);

			if (uniformState)
			{
				globalState->allState = tempState;
			}
			else
			{
				globalState->allState = D3D12_RESOURCE_STATE_UNKNOWN;
			}
		}
	}

	void Texture::transition(std::vector<D3D12_RESOURCE_BARRIER>& resourceBarriers, std::vector<D3D12ResourceBase*>& pendingResources)
	{
		//检查transitionState的每个mipslice的状态
		//如果它们都是相同的，那么把allState设置为那个状态
		if (transitionState->allState == D3D12_RESOURCE_STATE_UNKNOWN)
		{
			const uint32_t tempState = transitionState->mipLevelStates[0];

			const bool uniformState = ((tempState == D3D12_RESOURCE_STATE_UNKNOWN) ? false : transitionState->allOfEqual(tempState));

			if (uniformState)
			{
				transitionState->allState = tempState;
			}
		}

		bool finalStateChecking = false;

		//如果transitionState的allState是已知的
		if (transitionState->allState != D3D12_RESOURCE_STATE_UNKNOWN)
		{
			//如果纹理有多个mipslice
			if (mipLevels > 1)
			{
				//这是最好的情况，transitionState和internalState的allState都是已知的
				if (internalState->allState != D3D12_RESOURCE_STATE_UNKNOWN)
				{
					if (!Utils::Math::bitFlagSubset(internalState->allState, transitionState->allState))
					{
						D3D12_RESOURCE_BARRIER barrier = {};
						barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
						barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
						barrier.Transition.pResource = getResource();
						barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
						barrier.Transition.StateBefore = static_cast<D3D12_RESOURCE_STATES>(internalState->allState);
						barrier.Transition.StateAfter = static_cast<D3D12_RESOURCE_STATES>(transitionState->allState);

						resourceBarriers.push_back(barrier);

						internalState->set(transitionState->allState);
					}
					else if (internalState->allState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS && transitionState->allState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
					{
						resourceBarriers.push_back(CD3DX12_RESOURCE_BARRIER::UAV(getResource()));
					}
				}
				//internalState的allState是未知的，那么需要进一步检查
				else
				{
					//首先检查internalState的所有mipslice的状态是否都是未知的
					const bool allStatesUnknown = internalState->allOfEqual(D3D12_RESOURCE_STATE_UNKNOWN);

					//如果internalState的所有mipslice的状态都是未知的，那么我们需要待定资源屏障
					if (allStatesUnknown)
					{
						pendingState->set(transitionState->allState);

						internalState->set(transitionState->allState);

						pushToPendingList(pendingResources);
					}
					//internalState的有些mipslice的状态是已知的
					else
					{
						bool insertUAVBarrier = false;

						//对于未知的内部状态我们需要设置Pending State
						//对于已知的内部状态我们需要D3D12_RESOURCE_BARRIER
						for (uint32_t mipSlice = 0; mipSlice < mipLevels; mipSlice++)
						{
							//未知
							if (internalState->mipLevelStates[mipSlice] == D3D12_RESOURCE_STATE_UNKNOWN)
							{
								finalStateChecking = true;

								pendingState->mipLevelStates[mipSlice] = transitionState->mipLevelStates[mipSlice];

								internalState->mipLevelStates[mipSlice] = transitionState->mipLevelStates[mipSlice];

								pushToPendingList(pendingResources);
							}
							//已知
							else
							{
								if (!Utils::Math::bitFlagSubset(internalState->mipLevelStates[mipSlice], transitionState->mipLevelStates[mipSlice]))
								{
									finalStateChecking = true;

									for (uint32_t arraySlice = 0; arraySlice < arraySize; arraySlice++)
									{
										D3D12_RESOURCE_BARRIER barrier = {};
										barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
										barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
										barrier.Transition.pResource = getResource();
										barrier.Transition.Subresource = D3D12CalcSubresource(mipSlice, arraySlice, 0, mipLevels, arraySize);
										barrier.Transition.StateBefore = static_cast<D3D12_RESOURCE_STATES>(internalState->mipLevelStates[mipSlice]);
										barrier.Transition.StateAfter = static_cast<D3D12_RESOURCE_STATES>(transitionState->mipLevelStates[mipSlice]);

										resourceBarriers.push_back(barrier);
									}

									internalState->mipLevelStates[mipSlice] = transitionState->mipLevelStates[mipSlice];
								}
								else if (!insertUAVBarrier && internalState->mipLevelStates[mipSlice] == D3D12_RESOURCE_STATE_UNORDERED_ACCESS && transitionState->mipLevelStates[mipSlice] == D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
								{
									resourceBarriers.push_back(CD3DX12_RESOURCE_BARRIER::UAV(getResource()));

									insertUAVBarrier = true;
								}
							}
						}
					}
				}
			}
			//如果纹理只有一个mipslice
			else
			{
				//未知
				if (internalState->allState == D3D12_RESOURCE_STATE_UNKNOWN)
				{
					pendingState->set(transitionState->allState);

					internalState->set(transitionState->allState);

					pushToPendingList(pendingResources);
				}
				else
				{
					if (!Utils::Math::bitFlagSubset(internalState->allState, transitionState->allState))
					{
						D3D12_RESOURCE_BARRIER barrier = {};
						barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
						barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
						barrier.Transition.pResource = getResource();
						barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
						barrier.Transition.StateBefore = static_cast<D3D12_RESOURCE_STATES>(internalState->allState);
						barrier.Transition.StateAfter = static_cast<D3D12_RESOURCE_STATES>(transitionState->allState);

						resourceBarriers.push_back(barrier);

						internalState->set(transitionState->allState);
					}
					else if (internalState->allState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS && transitionState->allState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
					{
						resourceBarriers.push_back(CD3DX12_RESOURCE_BARRIER::UAV(getResource()));
					}
				}
			}
		}
		//如果transitionState的allState是未知的
		//在这种情况下我们需要检查internalState和transitionState的各个mipslice的状态
		else
		{
			if (mipLevels > 1)
			{
				bool insertUAVBarrier = false;

				for (uint32_t mipSlice = 0; mipSlice < mipLevels; mipSlice++)
				{
					//检查transitionState的每个mipslice的状态，只有已知情况才进行状态转变
					if (transitionState->mipLevelStates[mipSlice] != D3D12_RESOURCE_STATE_UNKNOWN)
					{
						//未知
						if (internalState->mipLevelStates[mipSlice] == D3D12_RESOURCE_STATE_UNKNOWN)
						{
							finalStateChecking = true;

							pendingState->mipLevelStates[mipSlice] = transitionState->mipLevelStates[mipSlice];

							internalState->mipLevelStates[mipSlice] = transitionState->mipLevelStates[mipSlice];

							pushToPendingList(pendingResources);
						}
						//已知
						else
						{
							if (!Utils::Math::bitFlagSubset(internalState->mipLevelStates[mipSlice], transitionState->mipLevelStates[mipSlice]))
							{
								finalStateChecking = true;

								for (uint32_t arraySlice = 0; arraySlice < arraySize; arraySlice++)
								{
									D3D12_RESOURCE_BARRIER barrier = {};
									barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
									barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
									barrier.Transition.pResource = getResource();
									barrier.Transition.Subresource = D3D12CalcSubresource(mipSlice, arraySlice, 0, mipLevels, arraySize);
									barrier.Transition.StateBefore = static_cast<D3D12_RESOURCE_STATES>(internalState->mipLevelStates[mipSlice]);
									barrier.Transition.StateAfter = static_cast<D3D12_RESOURCE_STATES>(transitionState->mipLevelStates[mipSlice]);

									resourceBarriers.push_back(barrier);
								}

								internalState->mipLevelStates[mipSlice] = transitionState->mipLevelStates[mipSlice];
							}
							else if (!insertUAVBarrier && internalState->mipLevelStates[mipSlice] == D3D12_RESOURCE_STATE_UNORDERED_ACCESS && transitionState->mipLevelStates[mipSlice] == D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
							{
								resourceBarriers.push_back(CD3DX12_RESOURCE_BARRIER::UAV(getResource()));

								insertUAVBarrier = true;
							}
						}
					}
				}
			}
			else
			{
				LOGERROR(L"当转变只有一个miplevel的纹理时，它的allState必须是已知的！");
			}
		}

		//之前的操作可能会让纹理的内部状态的每个mipslice的状态相同
		//因此我们需要进行检查，如果所有mipslice的状态相同，那么要设置internalState的allState为那个状态
		if (finalStateChecking)
		{
			const uint32_t tempState = internalState->mipLevelStates[0];

			const bool uniformState = ((tempState == D3D12_RESOURCE_STATE_UNKNOWN) ? false : internalState->allOfEqual(tempState));

			if (uniformState)
			{
				internalState->allState = tempState;
			}
			else
			{
				internalState->allState = D3D12_RESOURCE_STATE_UNKNOWN;
			}
		}
	}

	void Texture::resolvePendingState(std::vector<D3D12_RESOURCE_BARRIER>& resourceBarriers)
	{
		//检查pendingState的每个mipslice的状态
		//如果它们都是相同的，那么把allState设置为那个状态
		if (pendingState->allState == D3D12_RESOURCE_STATE_UNKNOWN)
		{
			const uint32_t tempState = pendingState->mipLevelStates[0];

			const bool uniformState = ((tempState == D3D12_RESOURCE_STATE_UNKNOWN) ? false : pendingState->allOfEqual(tempState));

			if (uniformState)
			{
				pendingState->allState = tempState;
			}
		}

		//如果pendingState的allState是已知的
		if (pendingState->allState != D3D12_RESOURCE_STATE_UNKNOWN)
		{
			//如果纹理有多个mipslice
			if (mipLevels > 1)
			{
				//这是最好的情况，pendingState和globalState的allState都是已知的
				if (globalState->allState != D3D12_RESOURCE_STATE_UNKNOWN)
				{
					if (globalState->allState != pendingState->allState)
					{
						D3D12_RESOURCE_BARRIER barrier = {};
						barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
						barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
						barrier.Transition.pResource = getResource();
						barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
						barrier.Transition.StateBefore = static_cast<D3D12_RESOURCE_STATES>(globalState->allState);
						barrier.Transition.StateAfter = static_cast<D3D12_RESOURCE_STATES>(pendingState->allState);

						resourceBarriers.push_back(barrier);
					}
					else if (globalState->allState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS && pendingState->allState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
					{
						resourceBarriers.push_back(CD3DX12_RESOURCE_BARRIER::UAV(getResource()));
					}
				}
				//globalState的allState是未知的，那么需要进一步检查
				else
				{
					bool insertUAVBarrier = false;

					for (uint32_t mipSlice = 0; mipSlice < mipLevels; mipSlice++)
					{
						//检测到状态不相等就进行转换
						if (globalState->mipLevelStates[mipSlice] != pendingState->mipLevelStates[mipSlice])
						{
							for (uint32_t arraySlice = 0; arraySlice < arraySize; arraySlice++)
							{
								D3D12_RESOURCE_BARRIER barrier = {};
								barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
								barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
								barrier.Transition.pResource = getResource();
								barrier.Transition.Subresource = D3D12CalcSubresource(mipSlice, arraySlice, 0, mipLevels, arraySize);
								barrier.Transition.StateBefore = static_cast<D3D12_RESOURCE_STATES>(globalState->mipLevelStates[mipSlice]);
								barrier.Transition.StateAfter = static_cast<D3D12_RESOURCE_STATES>(pendingState->mipLevelStates[mipSlice]);

								resourceBarriers.push_back(barrier);
							}
						}
						else if (!insertUAVBarrier && globalState->mipLevelStates[mipSlice] == D3D12_RESOURCE_STATE_UNORDERED_ACCESS && pendingState->mipLevelStates[mipSlice] == D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
						{
							resourceBarriers.push_back(CD3DX12_RESOURCE_BARRIER::UAV(getResource()));

							insertUAVBarrier = true;
						}

					}
				}
			}
			//如果纹理只有一个mipslice
			else
			{
				if (globalState->allState != pendingState->allState)
				{
					D3D12_RESOURCE_BARRIER barrier = {};
					barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
					barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
					barrier.Transition.pResource = getResource();
					barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
					barrier.Transition.StateBefore = static_cast<D3D12_RESOURCE_STATES>(globalState->allState);
					barrier.Transition.StateAfter = static_cast<D3D12_RESOURCE_STATES>(pendingState->allState);

					resourceBarriers.push_back(barrier);
				}
				else if (globalState->allState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS && pendingState->allState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
				{
					resourceBarriers.push_back(CD3DX12_RESOURCE_BARRIER::UAV(getResource()));
				}
			}
		}
		//如果pendingState的allState是未知的
		//在这种情况下我们需要检查pendingState的各个mipslice的状态
		else
		{
			if (mipLevels > 1)
			{
				bool insertUAVBarrier = false;

				for (uint32_t mipSlice = 0; mipSlice < mipLevels; mipSlice++)
				{
					//检查pendingState的每个mipslice的状态，只有已知情况才进行状态转变
					if (pendingState->mipLevelStates[mipSlice] != D3D12_RESOURCE_STATE_UNKNOWN)
					{
						if (globalState->mipLevelStates[mipSlice] != pendingState->mipLevelStates[mipSlice])
						{
							for (uint32_t arraySlice = 0; arraySlice < arraySize; arraySlice++)
							{
								D3D12_RESOURCE_BARRIER barrier = {};
								barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
								barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
								barrier.Transition.pResource = getResource();
								barrier.Transition.Subresource = D3D12CalcSubresource(mipSlice, arraySlice, 0, mipLevels, arraySize);
								barrier.Transition.StateBefore = static_cast<D3D12_RESOURCE_STATES>(globalState->mipLevelStates[mipSlice]);
								barrier.Transition.StateAfter = static_cast<D3D12_RESOURCE_STATES>(pendingState->mipLevelStates[mipSlice]);

								resourceBarriers.push_back(barrier);
							}
						}
						else if (!insertUAVBarrier && globalState->mipLevelStates[mipSlice] == D3D12_RESOURCE_STATE_UNORDERED_ACCESS && pendingState->mipLevelStates[mipSlice] == D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
						{
							resourceBarriers.push_back(CD3DX12_RESOURCE_BARRIER::UAV(getResource()));

							insertUAVBarrier = true;
						}
					}
				}
			}
			else
			{
				LOGERROR(L"当转变只有一个miplevel的纹理时，它的allState必须是已知的！");
			}
		}
	}

	void Texture::resetInternalState()
	{
		internalState->reset();
	}

	void Texture::resetTransitionState()
	{
		transitionState->reset();
	}

	void Texture::resetPendingState()
	{
		pendingState->reset();
	}

	uint32_t Texture::getWidth() const
	{
		return width;
	}

	uint32_t Texture::getHeight() const
	{
		return height;
	}

	uint32_t Texture::getArraySize() const
	{
		return arraySize;
	}

	uint32_t Texture::getMipLevels() const
	{
		return mipLevels;
	}

	DXGI_FORMAT Texture::getFormat() const
	{
		return format;
	}

	void Texture::setAllState(const uint32_t state)
	{
		if (transitionState->allState == D3D12_RESOURCE_STATE_UNKNOWN)
		{
			if (mipLevels > 1)
			{
				transitionState->combine(state);
			}
			else
			{
				transitionState->set(state);
			}
		}
		else if (!Utils::Math::bitFlagSubset(internalState->allState, state))
		{
			transitionState->combineSimple(state);
		}
	}

	void Texture::setMipSliceState(const uint32_t mipSlice, const uint32_t state)
	{
		if (mipLevels > 1)
		{
			if (transitionState->mipLevelStates[mipSlice] == D3D12_RESOURCE_STATE_UNKNOWN)
			{
				transitionState->allState = D3D12_RESOURCE_STATE_UNKNOWN;

				transitionState->mipLevelStates[mipSlice] = state;
			}
			else if (!Utils::Math::bitFlagSubset(transitionState->mipLevelStates[mipSlice], state))
			{
				transitionState->allState = D3D12_RESOURCE_STATE_UNKNOWN;

				transitionState->mipLevelStates[mipSlice] = (transitionState->mipLevelStates[mipSlice] | state);
			}
		}
		else
		{
			if (transitionState->mipLevelStates[mipSlice] == D3D12_RESOURCE_STATE_UNKNOWN)
			{
				transitionState->set(state);
			}
			else
			{
				transitionState->combineSimple(state);
			}
		}
	}

	Texture::States::States(const uint32_t initialState, const uint32_t mipLevels) :
		mipLevels(mipLevels), mipLevelStates(makeUnique<uint32_t[]>(mipLevels))
	{
		set(initialState);
	}

	Texture::States::~States()
	{
	}

	void Texture::States::set(const uint32_t state) noexcept
	{
		allState = state;

		for (uint32_t i = 0; i < mipLevels; i++)
		{
			mipLevelStates[i] = state;
		}
	}

	void Texture::States::reset() noexcept
	{
		set(D3D12_RESOURCE_STATE_UNKNOWN);
	}

	void Texture::States::combineSimple(const uint32_t state) noexcept
	{
		allState = (allState | state);

		for (uint32_t i = 0; i < mipLevels; i++)
		{
			mipLevelStates[i] = mipLevelStates[i] | state;
		}
	}

	void Texture::States::combine(const uint32_t state) noexcept
	{
		for (uint32_t i = 0; i < mipLevels; i++)
		{
			if (mipLevelStates[i] == D3D12_RESOURCE_STATE_UNKNOWN)
			{
				mipLevelStates[i] = state;
			}
			else
			{
				mipLevelStates[i] = mipLevelStates[i] | state;
			}
		}
	}

	bool Texture::States::allOfEqual(const uint32_t state) const noexcept
	{
		for (uint32_t i = 0; i < mipLevels; i++)
		{
			if (state != mipLevelStates[i])
			{
				return false;
			}
		}

		return true;
	}
}