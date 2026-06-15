#pragma once

#include<Gear/DevEssential.h>

class TextBatch
{
public:

	TextBatch(ResourceManager& resManager, const std::wstring& filePath);

	void drawText(const wchar_t ch, const float x, const float y, const float z, const float r, const float g, const float b, const float a);

	void render(GraphicsContext& context, RenderTextureView& outputTexture);

	float getFontSize() const;

private:

	ShaderPtr spriteVS;

	ShaderPtr spriteGS;

	ShaderPtr spritePS;

	PipelineStatePtr spriteState;

	RenderTextureViewPtr fontTexture;

	BufferViewPtr textBuffer;

	//字体名
	std::wstring fontName;

	float currentFontSize;

	float fontSize;

	float scale;

	struct Text
	{
		DirectX::XMFLOAT3 position;

		DirectX::XMFLOAT2 size;

		DirectX::XMFLOAT4 color;

		float uvLeft;

		float uvRight;

		float uvBottom;

		float uvTop;
	};

	static constexpr size_t maxTextNum = 65536;

	StaticVector<Text, maxTextNum> textArray;

	struct Character
	{
		int xoffset;
		int yoffset;
		int xadvance;
		float width;
		float height;
		float leftTexCoord;
		float rightTexCoord;
		float bottomTexCoord;
		float topTexCoord;
	};

	std::unordered_map<wchar_t, const Character> characterMap;

};