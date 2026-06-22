#include "TextBatch.h"

#include<fstream>

std::vector<std::wstring> split(std::wstring str, const wchar_t separator)
{
	std::vector<std::wstring> result;

	size_t splitFrontEndIdx = str.find_first_of(separator);

	while (std::wstring::npos != splitFrontEndIdx)
	{
		const std::wstring splitFront = str.substr(0ull, splitFrontEndIdx);

		result.emplace_back(splitFront);

		const size_t splitBackBeginIdx = str.find_first_not_of(separator, splitFrontEndIdx);

		if (std::wstring::npos == splitBackBeginIdx)
		{
			return result;
		}

		const std::wstring splitBack = str.substr(splitBackBeginIdx);

		str = splitBack;

		splitFrontEndIdx = str.find_first_of(separator);
	}

	result.emplace_back(str);

	return result;
}

TextBatch::TextBatch(ResourceManager& resManager, const std::wstring& filePath) :
	spriteVS(Shader::create(File::getRootFolder() + L"SpriteVS.cso")),
	spriteGS(Shader::create(File::getRootFolder() + L"SpriteGS.cso")),
	spritePS(Shader::create(File::getRootFolder() + L"SpritePS.cso")),
	textBuffer(ResourceManager::createStructuredBufferView(textArray.elementByteSize(), textArray.totalByteSize(), false, false, true, true, true)),
	scale(1.f)
{
	spriteState = PipelineStateBuilder()
		.setBlendState(PipelineStateHelper::blendDefault)
		.setRasterizerState(PipelineStateHelper::rasterCullBack)
		.setDepthStencilState(PipelineStateHelper::depthCompareNone)
		.setPrimitiveTopologyType(TOPOLOGY::TYPE::POINT)
		.setRTVFormats({ FMT::RGBA16F })
		.setVS(*spriteVS)
		.setGS(*spriteGS)
		.setPS(*spritePS)
		.build();

	std::wifstream fileStream;

	fileStream.open(filePath);

	if (fileStream.bad())
	{
		LOGERROR(L"字体描述文件打开失败");
	}

	std::wstring str;

	fileStream >> str;

	while (str != L"kernings")
	{
		const std::vector<std::wstring> splitResult = split(str, L'=');

		if (splitResult.front() == L"size")
		{
			fontSize = std::stoi(splitResult.back());

			currentFontSize = fontSize;

			LOGUSER(L"字体大小", fontSize);
		}

		if (splitResult.front() == L"file")
		{
			std::wstring fontTexturePath = splitResult.back();

			fontTexturePath = fontTexturePath.substr(1ull, fontTexturePath.length() - 2ull);

			fontTexture = resManager.createRenderTextureView(fontTexturePath, true);

			break;
		}

		fileStream >> str;
	}

	while (str != L"kernings")
	{
		const std::vector<std::wstring> splitResult = split(str, L'=');

		if (splitResult.front() == L"id")
		{
			uint32_t id = static_cast<uint32_t>(std::stoi(splitResult.back()));

			int startX, startY, width, height, xoffset, yoffset, xadvance;

			for (uint32_t i = 0; i < 7; i++)
			{
				fileStream >> str;

				std::vector<std::wstring> parameter = split(str, L'=');

				if (parameter.front() == L"x")
				{
					startX = std::stoi(parameter.back());
				}
				else if (parameter.front() == L"y")
				{
					startY = std::stoi(parameter.back());
				}
				else if (parameter.front() == L"width")
				{
					width = std::stoi(parameter.back());
				}
				else if (parameter.front() == L"height")
				{
					height = std::stoi(parameter.back());
				}
				else if (parameter.front() == L"xoffset")
				{
					xoffset = std::stoi(parameter.back());
				}
				else if (parameter.front() == L"yoffset")
				{
					yoffset = std::stoi(parameter.back());
				}
				else if (parameter.front() == L"xadvance")
				{
					xadvance = std::stoi(parameter.back());
				}
			}

			const float bitmapWidth = static_cast<float>(fontTexture->get2Dimension().x);

			const float bitmapHeight = static_cast<float>(fontTexture->get2Dimension().y);

			const Character character{
				xoffset,
				-yoffset - height,
				xadvance,
				static_cast<float>(width),
				static_cast<float>(height),
				startX / static_cast<float>(bitmapWidth),
				(startX + width) / static_cast<float>(bitmapWidth),
				(startY + height) / static_cast<float>(bitmapHeight),
				startY / static_cast<float>(bitmapHeight)
			};

			characterMap.insert(std::pair<wchar_t, const Character>(static_cast<wchar_t>(id), character));

			//LOGUSER(id, startX, startY, width, height, xoffset, yoffset, xadvance);
		}

		fileStream >> str;
	}

	fileStream.close();
}

void TextBatch::drawText(const wchar_t ch, const float x, const float y, const float z, const float r, const float g, const float b, const float a)
{
	const Character& character = characterMap[ch];

	Text text;

	text.position = DirectX::XMFLOAT3(x, y, z);
	text.size = DirectX::XMFLOAT2(character.width * scale, character.height * scale);
	text.color = DirectX::XMFLOAT4(r, g, b, a);
	text.uvLeft = character.leftTexCoord;
	text.uvRight = character.rightTexCoord;
	text.uvBottom = character.bottomTexCoord;
	text.uvTop = character.topTexCoord;

	textArray.push(text);
}

void TextBatch::render(GraphicsContext& context, RenderTextureView& outputTexture)
{
	context.updateBuffer(*textBuffer, textArray.data(), textArray.byteSize());

	context.setPipelineState(*spriteState);

	context.setRenderTargets({ outputTexture.getRTVMipHandle(0) });

	context.setViewportSimple(outputTexture.get2Dimension());

	context.setPrimitiveTopology(TOPOLOGY::POINTLIST);

	context.setVertexBuffers(0, { textBuffer->getVertexBuffer() });

	SETCONSTS({
	context.setPSConstants({fontTexture->getAllSRVIndex()},co);
		});

	context.draw(textArray.size(), 1u, 0u, 0u);

	textArray.clear();
}

float TextBatch::getFontSize() const
{
	return fontSize;
}
