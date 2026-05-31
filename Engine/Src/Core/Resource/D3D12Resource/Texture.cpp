#include<Gear/Core/Resource/D3D12Resource/Texture.h>

Gear::Core::Resource::D3D12Resource::Texture::Texture(const uint32_t width, const uint32_t height, const DXGI_FORMAT format, const uint32_t arraySize, const uint32_t mipLevels, const bool stateTracking, const D3D12_RESOURCE_FLAGS resFlags, const D3D12_CLEAR_VALUE* const clearValue) :
	D3D12ResourceBase(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, CD3DX12_RESOURCE_DESC::Tex2D(format, width, height, arraySize, mipLevels, 1, 0, resFlags), stateTracking, D3D12_RESOURCE_STATE_COPY_DEST, clearValue),
	width(width),
	height(height),
	arraySize(arraySize),
	mipLevels(mipLevels),
	format(format),
	globalState(makeShared<States>(D3D12_RESOURCE_STATE_COPY_DEST, mipLevels)),
	internalState(new States(D3D12_RESOURCE_STATE_COPY_DEST, mipLevels)),
	transitionState(new States(D3D12_RESOURCE_STATE_UNKNOWN, mipLevels))
{

}

Gear::Core::Resource::D3D12Resource::Texture::Texture(const ComPtr<ID3D12Resource>& texture, const bool stateTracking, const uint32_t initialState) :
	D3D12ResourceBase(texture, stateTracking)
{
	D3D12_RESOURCE_DESC desc = getResource()->GetDesc();
	width = static_cast<uint32_t>(desc.Width);
	height = desc.Height;
	arraySize = desc.DepthOrArraySize;
	mipLevels = desc.MipLevels;
	format = desc.Format;

	globalState = makeShared<States>(initialState, mipLevels);
	internalState = new States(initialState, mipLevels);
	transitionState = new States(D3D12_RESOURCE_STATE_UNKNOWN, mipLevels);
}

Gear::Core::Resource::D3D12Resource::Texture::Texture(Texture& tex) :
	D3D12ResourceBase(tex),
	width(tex.width),
	height(tex.height),
	arraySize(tex.arraySize),
	mipLevels(tex.mipLevels),
	format(tex.format),
	globalState(tex.globalState),
	internalState(new States(D3D12_RESOURCE_STATE_UNKNOWN, mipLevels)),
	transitionState(new States(D3D12_RESOURCE_STATE_UNKNOWN, mipLevels))
{
	tex.resetInternalStates();
}

Gear::Core::Resource::D3D12Resource::Texture::~Texture()
{
	if (internalState)
	{
		delete internalState;
	}

	if (transitionState)
	{
		delete transitionState;
	}
}

void Gear::Core::Resource::D3D12Resource::Texture::updateGlobalStates()
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

void Gear::Core::Resource::D3D12Resource::Texture::resetInternalStates()
{
	internalState->reset();
}

void Gear::Core::Resource::D3D12Resource::Texture::resetTransitionStates()
{
	transitionState->reset();
}

void Gear::Core::Resource::D3D12Resource::Texture::transition(std::vector<D3D12_RESOURCE_BARRIER>& transitionBarriers, std::vector<PendingTextureBarrier>& pendingBarriers)
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
				if (!bitFlagSubset(internalState->allState, transitionState->allState))
				{
					D3D12_RESOURCE_BARRIER barrier = {};
					barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
					barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
					barrier.Transition.pResource = getResource();
					barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
					barrier.Transition.StateBefore = static_cast<D3D12_RESOURCE_STATES>(internalState->allState);
					barrier.Transition.StateAfter = static_cast<D3D12_RESOURCE_STATES>(transitionState->allState);

					transitionBarriers.push_back(barrier);

					internalState->set(transitionState->allState);
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
					PendingTextureBarrier barrier = {};
					barrier.texture = this;
					barrier.mipSlice = D3D12_TRANSITION_ALL_MIPLEVELS;
					barrier.afterState = transitionState->allState;

					pendingBarriers.push_back(barrier);

					internalState->set(transitionState->allState);
				}
				//internalState的有些mipslice的状态是已知的
				else
				{
					//对于未知的内部状态我们需要PendingTextureBarrier
					//对于已知的内部状态我们需要D3D12_RESOURCE_BARRIER
					for (uint32_t mipSlice = 0; mipSlice < mipLevels; mipSlice++)
					{
						//未知
						if (internalState->mipLevelStates[mipSlice] == D3D12_RESOURCE_STATE_UNKNOWN)
						{
							finalStateChecking = true;

							PendingTextureBarrier barrier = {};
							barrier.texture = this;
							barrier.mipSlice = mipSlice;
							barrier.afterState = transitionState->mipLevelStates[mipSlice];

							pendingBarriers.push_back(barrier);

							internalState->mipLevelStates[mipSlice] = transitionState->mipLevelStates[mipSlice];
						}
						//已知
						else
						{
							if (!bitFlagSubset(internalState->mipLevelStates[mipSlice], transitionState->mipLevelStates[mipSlice]))
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

									transitionBarriers.push_back(barrier);
								}

								internalState->mipLevelStates[mipSlice] = transitionState->mipLevelStates[mipSlice];
							}
						}
					}
				}
			}
		}
		//如果纹理只有一个mipslice
		else
		{
			if (internalState->allState == D3D12_RESOURCE_STATE_UNKNOWN)
			{
				PendingTextureBarrier barrier = {};
				barrier.texture = this;
				barrier.mipSlice = D3D12_TRANSITION_ALL_MIPLEVELS;
				barrier.afterState = transitionState->allState;

				pendingBarriers.push_back(barrier);

				internalState->set(transitionState->allState);
			}
			else
			{
				if (!bitFlagSubset(internalState->allState, transitionState->allState))
				{
					D3D12_RESOURCE_BARRIER barrier = {};
					barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
					barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
					barrier.Transition.pResource = getResource();
					barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
					barrier.Transition.StateBefore = static_cast<D3D12_RESOURCE_STATES>(internalState->allState);
					barrier.Transition.StateAfter = static_cast<D3D12_RESOURCE_STATES>(transitionState->allState);

					transitionBarriers.push_back(barrier);

					internalState->set(transitionState->allState);
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
			for (uint32_t mipSlice = 0; mipSlice < mipLevels; mipSlice++)
			{
				//检查transitionState的每个mipslice的状态，只有已知情况才进行状态转变
				if (transitionState->mipLevelStates[mipSlice] != D3D12_RESOURCE_STATE_UNKNOWN)
				{
					//未知
					if (internalState->mipLevelStates[mipSlice] == D3D12_RESOURCE_STATE_UNKNOWN)
					{
						finalStateChecking = true;

						PendingTextureBarrier barrier = {};
						barrier.texture = this;
						barrier.mipSlice = mipSlice;
						barrier.afterState = transitionState->mipLevelStates[mipSlice];

						pendingBarriers.push_back(barrier);

						internalState->mipLevelStates[mipSlice] = transitionState->mipLevelStates[mipSlice];
					}
					//已知
					else
					{
						if (!bitFlagSubset(internalState->mipLevelStates[mipSlice], transitionState->mipLevelStates[mipSlice]))
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

								transitionBarriers.push_back(barrier);
							}

							internalState->mipLevelStates[mipSlice] = transitionState->mipLevelStates[mipSlice];
						}
					}
				}
			}
		}
		else
		{
			LOGERROR(L"when transition texture that has only 1 miplevel,its allState must be known");
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

	//最后要重置transitionState用于后续的使用
	resetTransitionStates();
}

void Gear::Core::Resource::D3D12Resource::Texture::solvePendingBarrier(std::vector<D3D12_RESOURCE_BARRIER>& transitionBarriers, const uint32_t targetMipSlice, const uint32_t targetState)
{
	if (targetMipSlice == D3D12_TRANSITION_ALL_MIPLEVELS)
	{
		if (mipLevels > 1)
		{
			if (globalState->allState != D3D12_RESOURCE_STATE_UNKNOWN)
			{
				if (globalState->allState != targetState)
				{
					D3D12_RESOURCE_BARRIER barrier = {};
					barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
					barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
					barrier.Transition.pResource = getResource();
					barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
					barrier.Transition.StateBefore = static_cast<D3D12_RESOURCE_STATES>(globalState->allState);
					barrier.Transition.StateAfter = static_cast<D3D12_RESOURCE_STATES>(targetState);

					transitionBarriers.push_back(barrier);
				}
			}
			else
			{
				for (uint32_t mipSlice = 0; mipSlice < mipLevels; mipSlice++)
				{
					if (globalState->mipLevelStates[mipSlice] != targetState)
					{
						for (uint32_t arraySlice = 0; arraySlice < arraySize; arraySlice++)
						{
							D3D12_RESOURCE_BARRIER barrier = {};
							barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
							barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
							barrier.Transition.pResource = getResource();
							barrier.Transition.Subresource = D3D12CalcSubresource(mipSlice, arraySlice, 0, mipLevels, arraySize);
							barrier.Transition.StateBefore = static_cast<D3D12_RESOURCE_STATES>(globalState->mipLevelStates[mipSlice]);
							barrier.Transition.StateAfter = static_cast<D3D12_RESOURCE_STATES>(targetState);

							transitionBarriers.push_back(barrier);
						}
					}
				}
			}
		}
		else
		{
			if (globalState->allState != targetState)
			{
				D3D12_RESOURCE_BARRIER barrier = {};
				barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				barrier.Transition.pResource = getResource();
				barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
				barrier.Transition.StateBefore = static_cast<D3D12_RESOURCE_STATES>(globalState->allState);
				barrier.Transition.StateAfter = static_cast<D3D12_RESOURCE_STATES>(targetState);

				transitionBarriers.push_back(barrier);
			}
		}
	}
	else
	{
		if (mipLevels > 1)
		{
			if (globalState->mipLevelStates[targetMipSlice] != targetState)
			{
				for (uint32_t arraySlice = 0; arraySlice < arraySize; arraySlice++)
				{
					D3D12_RESOURCE_BARRIER barrier = {};
					barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
					barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
					barrier.Transition.pResource = getResource();
					barrier.Transition.Subresource = D3D12CalcSubresource(targetMipSlice, arraySlice, 0, mipLevels, arraySize);
					barrier.Transition.StateBefore = static_cast<D3D12_RESOURCE_STATES>(globalState->mipLevelStates[targetMipSlice]);
					barrier.Transition.StateAfter = static_cast<D3D12_RESOURCE_STATES>(targetState);

					transitionBarriers.push_back(barrier);
				}
			}
		}
		else
		{
			LOGERROR(L"when transition a texture that has only 1 miplevel,pending mipslice must be D3D12_TRANSITION_ALL_MIPLEVELS");
		}
	}
}

uint32_t Gear::Core::Resource::D3D12Resource::Texture::getWidth() const
{
	return width;
}

uint32_t Gear::Core::Resource::D3D12Resource::Texture::getHeight() const
{
	return height;
}

uint32_t Gear::Core::Resource::D3D12Resource::Texture::getArraySize() const
{
	return arraySize;
}

uint32_t Gear::Core::Resource::D3D12Resource::Texture::getMipLevels() const
{
	return mipLevels;
}

DXGI_FORMAT Gear::Core::Resource::D3D12Resource::Texture::getFormat() const
{
	return format;
}

void Gear::Core::Resource::D3D12Resource::Texture::setAllState(const uint32_t state)
{
	if (transitionState->allState == D3D12_RESOURCE_STATE_UNKNOWN)
	{
		if (mipLevels > 1)
		{
			transitionState->forEach([state](uint32_t& element)
				{
					if (element == D3D12_RESOURCE_STATE_UNKNOWN)
					{
						element = state;
					}
					else
					{
						element = (element | state);
					}
				});
		}
		else
		{
			transitionState->set(state);
		}
	}
	else if (!bitFlagSubset(internalState->allState, state))
	{
		transitionState->combine(state);
	}
}

void Gear::Core::Resource::D3D12Resource::Texture::setMipSliceState(const uint32_t mipSlice, const uint32_t state)
{
	if (mipLevels > 1)
	{
		if (transitionState->mipLevelStates[mipSlice] == D3D12_RESOURCE_STATE_UNKNOWN)
		{
			transitionState->allState = D3D12_RESOURCE_STATE_UNKNOWN;

			transitionState->mipLevelStates[mipSlice] = state;
		}
		else if (!bitFlagSubset(transitionState->mipLevelStates[mipSlice], state))
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
			transitionState->combine(state);
		}
	}
}

uint32_t Gear::Core::Resource::D3D12Resource::Texture::getAllState() const
{
	return internalState->allState;
}

uint32_t Gear::Core::Resource::D3D12Resource::Texture::getMipSliceState(const uint32_t mipSlice) const
{
	return internalState->mipLevelStates[mipSlice];
}

void Gear::Core::Resource::D3D12Resource::Texture::pushToTrackingList(std::vector<Texture*>& trackingList)
{
	if (!getInTrackingList())
	{
		trackingList.push_back(this);

		D3D12ResourceBase::pushToTrackingList();
	}
}

Gear::Core::Resource::D3D12Resource::Texture::States::States(const uint32_t initialState, const uint32_t mipLevels) :
	mipLevels(mipLevels), mipLevelStates(new uint32_t[mipLevels])
{
	set(initialState);
}

Gear::Core::Resource::D3D12Resource::Texture::States::~States()
{
	if (mipLevelStates)
	{
		delete[] mipLevelStates;
	}
}

void Gear::Core::Resource::D3D12Resource::Texture::States::set(const uint32_t state)
{
	allState = state;

	std::fill(mipLevelStates, mipLevelStates + mipLevels, state);
}

void Gear::Core::Resource::D3D12Resource::Texture::States::reset()
{
	set(D3D12_RESOURCE_STATE_UNKNOWN);
}

void Gear::Core::Resource::D3D12Resource::Texture::States::combine(const uint32_t state)
{
	allState = (allState | state);

	forEach([state](uint32_t& element)
		{
			element = (element | state);
		});
}

bool Gear::Core::Resource::D3D12Resource::Texture::States::allOfEqual(const uint32_t state) const
{
	return std::all_of(mipLevelStates, mipLevelStates + mipLevels,
		[state](const uint32_t element)
		{
			return element == state;
		});
}
