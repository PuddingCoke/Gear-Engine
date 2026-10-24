#include<Gear/Core/D3D12Core/DXCCompiler.h>

#include<Gear/Core/D3D12Core/Internal/DXCCompilerInternal.h>

#include<Gear/Utils/File.h>

namespace Gear::Core::D3D12Core::DXCCompiler
{
	class DXCCompilerImpl
	{
	public:

		DXCCompilerImpl(const DXCCompilerImpl&) = delete;

		void operator=(const DXCCompilerImpl&) = delete;

		DXCCompilerImpl();

		~DXCCompilerImpl();

		//raw bytes
		ComPtr<IDxcBlob> load(const uint8_t* const bytes, const size_t byteSize);

		//hlsl
		ComPtr<IDxcBlob> compile(const Utils::File::Path& filePath, const ShaderType type) const;

		//cso
		ComPtr<IDxcBlob> read(const Utils::File::Path& filePath) const;

		ComPtr<ID3D12ShaderReflection> getReflectionBlob(const ComPtr<IDxcBlob>& shaderBlob) const;

	private:

		static constexpr uint32_t codePage = CP_UTF8;

		ComPtr<IDxcCompiler3> dxcCompiler;

		ComPtr<IDxcUtils> dxcUtils;

		ComPtr<IDxcIncludeHandler> dxcIncludeHanlder;

		ComPtr<IDxcContainerReflection> dxcContainerReflection;

	};

	DXCCompilerImpl::DXCCompilerImpl()
	{
		CHECKERROR(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler)));

		CHECKERROR(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils)));

		CHECKERROR(dxcUtils->CreateDefaultIncludeHandler(&dxcIncludeHanlder));

		CHECKERROR(DxcCreateInstance(CLSID_DxcContainerReflection, IID_PPV_ARGS(&dxcContainerReflection)));
	}

	DXCCompilerImpl::~DXCCompilerImpl()
	{
	}

	ComPtr<IDxcBlob> DXCCompilerImpl::load(const uint8_t* const bytes, const size_t byteSize)
	{
		ComPtr<IDxcBlobEncoding> textBlob;

		CHECKERROR(dxcUtils->CreateBlobFromPinned(bytes, static_cast<uint32_t>(byteSize), CP_NONE, &textBlob));

		ComPtr<IDxcBlob> shaderBlob = textBlob;

		return shaderBlob;
	}

	ComPtr<IDxcBlob> DXCCompilerImpl::compile(const Utils::File::Path& filePath, const ShaderType type) const
	{
		if (type == ShaderType::TYPECOUNT)
		{
			THROWLOG(LOGERROR() << TOSTRING(type) << "不得为" << TOSTRING(ShaderType::TYPECOUNT));
		}

		const std::vector<uint8_t> bytes = Utils::File::readAllBinary(filePath);

		ComPtr<IDxcBlobEncoding> textBlob;

		CHECKERROR(dxcUtils->CreateBlobFromPinned(bytes.data(), static_cast<uint32_t>(bytes.size()), codePage, &textBlob));

		DxcBuffer source = {};
		source.Ptr = textBlob->GetBufferPointer();
		source.Size = textBlob->GetBufferSize();
		source.Encoding = codePage;

		ComPtr<IDxcCompilerArgs> args;

		switch (type)
		{
		case ShaderType::VERTEX:
			dxcUtils->BuildArguments(filePath.cstr(), L"main", L"vs_6_6", nullptr, 0, nullptr, 0, &args);
			break;
		case ShaderType::HULL:
			dxcUtils->BuildArguments(filePath.cstr(), L"main", L"hs_6_6", nullptr, 0, nullptr, 0, &args);
			break;
		case ShaderType::DOMAIN:
			dxcUtils->BuildArguments(filePath.cstr(), L"main", L"ds_6_6", nullptr, 0, nullptr, 0, &args);
			break;
		case ShaderType::GEOMETRY:
			dxcUtils->BuildArguments(filePath.cstr(), L"main", L"gs_6_6", nullptr, 0, nullptr, 0, &args);
			break;
		case ShaderType::PIXEL:
			dxcUtils->BuildArguments(filePath.cstr(), L"main", L"ps_6_6", nullptr, 0, nullptr, 0, &args);
			break;
		case ShaderType::COMPUTE:
			dxcUtils->BuildArguments(filePath.cstr(), L"main", L"cs_6_6", nullptr, 0, nullptr, 0, &args);
			break;
		case ShaderType::AMPLIFICATION:
			dxcUtils->BuildArguments(filePath.cstr(), L"main", L"as_6_6", nullptr, 0, nullptr, 0, &args);
			break;
		case ShaderType::MESH:
			dxcUtils->BuildArguments(filePath.cstr(), L"main", L"ms_6_6", nullptr, 0, nullptr, 0, &args);
			break;
		case ShaderType::LIBRARY:
			dxcUtils->BuildArguments(filePath.cstr(), L"", L"lib_6_6", nullptr, 0, nullptr, 0, &args);
			break;
		default:
			break;
		}

		ComPtr<IDxcOperationResult> result;

		CHECKERROR(dxcCompiler->Compile(&source, args->GetArguments(), args->GetCount(), dxcIncludeHanlder.Get(), IID_PPV_ARGS(&result)));

		ComPtr<IDxcBlob> shaderBlob;

		CHECKERROR(result->GetResult(&shaderBlob));

		return shaderBlob;
	}

	ComPtr<IDxcBlob> DXCCompilerImpl::read(const Utils::File::Path& filePath) const
	{
		const std::vector<uint8_t> bytes = Utils::File::readAllBinary(filePath);

		ComPtr<IDxcBlobEncoding> textBlob;

		CHECKERROR(dxcUtils->CreateBlob(bytes.data(), static_cast<uint32_t>(bytes.size()), CP_NONE, &textBlob));

		ComPtr<IDxcBlob> shaderBlob = textBlob;

		return shaderBlob;
	}

	ComPtr<ID3D12ShaderReflection> DXCCompilerImpl::getReflectionBlob(const ComPtr<IDxcBlob>& shaderBlob) const
	{
		dxcContainerReflection->Load(shaderBlob.Get());

		uint32_t partIndex;

		ComPtr<ID3D12ShaderReflection> shaderReflection;

		CHECKERROR(dxcContainerReflection->FindFirstPartKind(DXC_PART_REFLECTION_DATA, &partIndex));

		ComPtr<IDxcBlob> reflectionBlob;

		CHECKERROR(dxcContainerReflection->GetPartContent(partIndex, &reflectionBlob));

		const DxcBuffer reflectionBuffer = { reflectionBlob->GetBufferPointer(),reflectionBlob->GetBufferSize(),0 };

		CHECKERROR(dxcUtils->CreateReflection(&reflectionBuffer, IID_PPV_ARGS(&shaderReflection)));

		return shaderReflection;
	}

	thread_local UniquePtr<DXCCompilerImpl> impl;

	namespace Internal
	{
		void initialize()
		{
			impl = makeUnique<DXCCompilerImpl>();
		}

		void release()
		{
			impl.reset();
		}
	}

	ComPtr<IDxcBlob> load(const uint8_t* const bytes, const size_t byteSize)
	{
		return impl->load(bytes, byteSize);
	}

	ComPtr<IDxcBlob> compile(const Utils::File::Path& filePath, const ShaderType type)
	{
		return impl->compile(filePath, type);
	}

	ComPtr<IDxcBlob> read(const Utils::File::Path& filePath)
	{
		return impl->read(filePath);
	}

	ComPtr<ID3D12ShaderReflection> getReflectionBlob(const ComPtr<IDxcBlob>& shaderBlob)
	{
		return impl->getReflectionBlob(shaderBlob);
	}
}
