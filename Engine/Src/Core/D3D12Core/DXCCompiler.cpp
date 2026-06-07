#include<Gear/Core/D3D12Core/DXCCompiler.h>

#include<Gear/Core/D3D12Core/Internal/DXCCompilerInternal.h>

#include<Gear/Utils/File.h>

namespace Gear::Core::D3D12Core::DXCCompiler
{
	namespace Internal
	{
		class DXCCompilerImpl
		{
		public:

			DXCCompilerImpl(const DXCCompilerImpl&) = delete;

			void operator=(const DXCCompilerImpl&) = delete;

			DXCCompilerImpl();

			~DXCCompilerImpl();

			//hlsl
			ComPtr<IDxcBlob> compile(const std::wstring& filePath, const ShaderProfile profile) const;

			//cso
			ComPtr<IDxcBlob> read(const std::wstring& filePath) const;

		private:

			static constexpr uint32_t codePage = CP_UTF8;

			ComPtr<IDxcCompiler3> dxcCompiler;

			ComPtr<IDxcUtils> dxcUtils;

			ComPtr<IDxcIncludeHandler> dxcIncludeHanlder;

		};

		DXCCompilerImpl::DXCCompilerImpl()
		{
			CHECKERROR(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler)));

			CHECKERROR(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils)));

			CHECKERROR(dxcUtils->CreateDefaultIncludeHandler(&dxcIncludeHanlder));
		}

		DXCCompilerImpl::~DXCCompilerImpl()
		{
		}

		ComPtr<IDxcBlob> DXCCompilerImpl::compile(const std::wstring& filePath, const ShaderProfile profile) const
		{
			const std::vector<uint8_t> bytes = Utils::File::readAllBinary(filePath);

			ComPtr<IDxcBlobEncoding> textBlob;

			CHECKERROR(dxcUtils->CreateBlobFromPinned(bytes.data(), static_cast<uint32_t>(bytes.size()), codePage, &textBlob));

			DxcBuffer source = {};
			source.Ptr = textBlob->GetBufferPointer();
			source.Size = textBlob->GetBufferSize();
			source.Encoding = codePage;

			ComPtr<IDxcCompilerArgs> args;

			switch (profile)
			{
			case ShaderProfile::VERTEX:
				dxcUtils->BuildArguments(filePath.c_str(), L"main", L"vs_6_6", nullptr, 0, nullptr, 0, &args);
				break;
			case ShaderProfile::HULL:
				dxcUtils->BuildArguments(filePath.c_str(), L"main", L"hs_6_6", nullptr, 0, nullptr, 0, &args);
				break;
			case ShaderProfile::DOMAIN:
				dxcUtils->BuildArguments(filePath.c_str(), L"main", L"ds_6_6", nullptr, 0, nullptr, 0, &args);
				break;
			case ShaderProfile::GEOMETRY:
				dxcUtils->BuildArguments(filePath.c_str(), L"main", L"gs_6_6", nullptr, 0, nullptr, 0, &args);
				break;
			case ShaderProfile::PIXEL:
				dxcUtils->BuildArguments(filePath.c_str(), L"main", L"ps_6_6", nullptr, 0, nullptr, 0, &args);
				break;
			case ShaderProfile::AMPLIFICATION:
				dxcUtils->BuildArguments(filePath.c_str(), L"main", L"as_6_6", nullptr, 0, nullptr, 0, &args);
				break;
			case ShaderProfile::MESH:
				dxcUtils->BuildArguments(filePath.c_str(), L"main", L"ms_6_6", nullptr, 0, nullptr, 0, &args);
				break;
			case ShaderProfile::COMPUTE:
				dxcUtils->BuildArguments(filePath.c_str(), L"main", L"cs_6_6", nullptr, 0, nullptr, 0, &args);
				break;
			case ShaderProfile::LIBRARY:
				dxcUtils->BuildArguments(filePath.c_str(), L"", L"lib_6_6", nullptr, 0, nullptr, 0, &args);
				break;
			default:
				LOGERROR(L"不被支持的着色器配置！");
				break;
			}

			ComPtr<IDxcOperationResult> result;

			CHECKERROR(dxcCompiler->Compile(&source, args->GetArguments(), args->GetCount(), dxcIncludeHanlder.Get(), IID_PPV_ARGS(&result)));

			ComPtr<IDxcBlob> shaderBlob;

			CHECKERROR(result->GetResult(&shaderBlob));

			return shaderBlob;
		}

		ComPtr<IDxcBlob> DXCCompilerImpl::read(const std::wstring& filePath) const
		{
			const std::vector<uint8_t> bytes = Utils::File::readAllBinary(filePath);

			ComPtr<IDxcBlobEncoding> textBlob;

			CHECKERROR(dxcUtils->CreateBlob(bytes.data(), static_cast<uint32_t>(bytes.size()), CP_NONE, &textBlob));

			ComPtr<IDxcBlob> shaderBlob = textBlob;

			return shaderBlob;
		}

		UniquePtr<DXCCompilerImpl> impl;

		void initialize()
		{
			impl = makeUnique<DXCCompilerImpl>();
		}

		void release()
		{
			impl.reset();
		}
	}

	ComPtr<IDxcBlob> compile(const std::wstring& filePath, const ShaderProfile profile)
	{
		return Internal::impl->compile(filePath, profile);
	}

	ComPtr<IDxcBlob> read(const std::wstring& filePath)
	{
		return Internal::impl->read(filePath);
	}
}
