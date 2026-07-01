#include<Gear/Gear2D/PrimitiveBatch.h>

#include<Gear/Core/PipelineStateHelper.h>

#include<Shaders/CompiledShaders/PrimitiveBatchLineVS.h>

#include<Shaders/CompiledShaders/PrimitiveBatchCircleVS.h>

#include<Shaders/CompiledShaders/PrimitiveBatchRCLineVS.h>

#include<Shaders/CompiledShaders/PrimitiveBatchRCLineGS.h>

#include<Shaders/CompiledShaders/PrimitiveBatchLineGS.h>

#include<Shaders/CompiledShaders/PrimitiveBatchPS.h>

//Gear::Core::Gear2D::PrimitiveBatch::PrimitiveBatch(const DXGI_FORMAT format, GraphicsContext* const context) :
//	context(context), lineWidth(1.f)
//{
//	lineVS = new D3D12Core::Shader(g_PrimitiveBatchLineVSBytes, sizeof(g_PrimitiveBatchLineVSBytes));
//
//	circleVS = new D3D12Core::Shader(g_PrimitiveBatchCircleVSBytes, sizeof(g_PrimitiveBatchCircleVSBytes));
//
//	rcLineVS = new D3D12Core::Shader(g_PrimitiveBatchRCLineVSBytes, sizeof(g_PrimitiveBatchRCLineVSBytes));
//
//	lineGS = new D3D12Core::Shader(g_PrimitiveBatchLineGSBytes, sizeof(g_PrimitiveBatchLineGSBytes));
//
//	rcLineGS = new D3D12Core::Shader(g_PrimitiveBatchRCLineGSBytes, sizeof(g_PrimitiveBatchRCLineGSBytes));
//
//	primitivePS = new D3D12Core::Shader(g_PrimitiveBatchPSBytes, sizeof(g_PrimitiveBatchPSBytes));
//
//	{
//		D3D12_INPUT_ELEMENT_DESC layout[] =
//		{
//			{"POSITION", 0, FMT::RG32F, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
//			{"COLOR", 0, FMT::RGBA32F, 0, 8, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
//		};
//
//		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = PipelineStateHelper::getDefaultGraphicsDesc();
//		desc.InputLayout = { layout,_countof(layout) };
//		desc.VS = lineVS->getByteCode();
//		desc.GS = lineGS->getByteCode();
//		desc.PS = primitivePS->getByteCode();
//		desc.RasterizerState = PipelineStateHelper::rasterCullBack;
//		desc.BlendState = PipelineStateHelper::blendDefault;
//		desc.DepthStencilState.DepthEnable = FALSE;
//		desc.DepthStencilState.StencilEnable = FALSE;
//		desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
//		desc.NumRenderTargets = 1;
//		desc.RTVFormats[0] = format;
//
//		GraphicsDevice::get()->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&lineState));
//	}
//
//	{
//		D3D12_INPUT_ELEMENT_DESC layout[] =
//		{
//			{"POSITION",0,FMT::RG32F,0,0,D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA,1},
//			{"POSITION",1,FMT::R32F,0,8,D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA,1},
//			{"COLOR",0,FMT::RGBA32F,0,12,D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA,1}
//		};
//
//		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = PipelineStateHelper::getDefaultGraphicsDesc();
//		desc.InputLayout = { layout,_countof(layout) };
//		desc.VS = circleVS->getByteCode();
//		desc.GS = lineGS->getByteCode();
//		desc.PS = primitivePS->getByteCode();
//		desc.RasterizerState = PipelineStateHelper::rasterCullBack;
//		desc.BlendState = PipelineStateHelper::blendDefault;
//		desc.DepthStencilState.DepthEnable = FALSE;
//		desc.DepthStencilState.StencilEnable = FALSE;
//		desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
//		desc.NumRenderTargets = 1;
//		desc.RTVFormats[0] = format;
//
//		GraphicsDevice::get()->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&circleState));
//	}
//
//	{
//		D3D12_INPUT_ELEMENT_DESC layout[] =
//		{
//			{"POSITION", 0, FMT::RG32F, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
//			{"POSITION", 1, FMT::R32F, 0, 8, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
//			{"COLOR", 0, FMT::RGBA32F, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
//		};
//
//		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = PipelineStateHelper::getDefaultGraphicsDesc();
//		desc.InputLayout = { layout,_countof(layout) };
//		desc.VS = rcLineVS->getByteCode();
//		desc.GS = rcLineGS->getByteCode();
//		desc.PS = primitivePS->getByteCode();
//		desc.RasterizerState = PipelineStateHelper::rasterCullBack;
//		desc.BlendState = PipelineStateHelper::blendDefault;
//		desc.DepthStencilState.DepthEnable = FALSE;
//		desc.DepthStencilState.StencilEnable = FALSE;
//		desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
//		desc.NumRenderTargets = 1;
//		desc.RTVFormats[0] = format;
//
//		GraphicsDevice::get()->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&roundCapLineState));
//	}
//}
//
//Gear::Core::Gear2D::PrimitiveBatch::~PrimitiveBatch()
//{
//	delete lineVS;
//	delete circleVS;
//	delete rcLineVS;
//	delete primitivePS;
//	delete lineGS;
//	delete rcLineGS;
//}
//
//void Gear::Core::Gear2D::PrimitiveBatch::begin()
//{
//	lineRenderer.begin();
//	circleRenderer.begin();
//	roundCapLineRenderer.begin();
//}
//
//void Gear::Core::Gear2D::PrimitiveBatch::end()
//{
//	context->setGSConstants(1, &lineWidth, 0);
//
//	lineRenderer.end(context, lineState.Get());
//	circleRenderer.end(context, circleState.Get());
//	roundCapLineRenderer.end(context, roundCapLineState.Get());
//}
//
//void Gear::Core::Gear2D::PrimitiveBatch::drawLine(const float x1, const float y1, const float x2, const float y2, const float r, const float g, const float b, const float a)
//{
//	lineRenderer.addLine(x1, y1, x2, y2, r, g, b, a);
//}
//
//void Gear::Core::Gear2D::PrimitiveBatch::drawCircle(const float x, const float y, const float length, const float r, const float g, const float b, const float a)
//{
//	circleRenderer.addCircle(x, y, length, r, g, b, a);
//}
//
//void Gear::Core::Gear2D::PrimitiveBatch::drawRoundCapLine(const float x1, const float y1, const float x2, const float y2, const float width, const float r, const float g, const float b, const float a)
//{
//	roundCapLineRenderer.addRoundCapLine(x1, y1, x2, y2, width, r, g, b, a);
//}
//
//void Gear::Core::Gear2D::PrimitiveBatch::setLineWidth(const float width)
//{
//	lineWidth = width;
//}
//
//Gear::Core::Gear2D::PrimitiveBatch::LineRenderer::LineRenderer() :
//	vertices(new float[2 * 6 * maxLineNum]), idx(0),
//	vertexBuffer(ResourceManager::createStructuredBufferView(sizeof(float) * 6, sizeof(float) * 6 * 2 * maxLineNum, false, false, true, true, true))
//{
//}
//
//Gear::Core::Gear2D::PrimitiveBatch::LineRenderer::~LineRenderer()
//{
//	delete[] vertices;
//	delete vertexBuffer;
//}
//
//void Gear::Core::Gear2D::PrimitiveBatch::LineRenderer::begin()
//{
//	idx = 0;
//}
//
//void Gear::Core::Gear2D::PrimitiveBatch::LineRenderer::end(GraphicsContext* const context, ID3D12PipelineState* const pipelineState)
//{
//	if (idx > 0)
//	{
//		context->updateBuffer(vertexBuffer, vertices, sizeof(float) * idx);
//
//		context->setPipelineState(pipelineState);
//
//		context->setPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
//
//		context->setVertexBuffers(0, { vertexBuffer->getVertexBuffer() });
//
//		context->flushResourceBarriers();
//
//		context->draw(idx / 6, 1, 0, 0);
//	}
//}
//
//void Gear::Core::Gear2D::PrimitiveBatch::LineRenderer::addLine(const float x1, const float y1, const float x2, const float y2, const float r, const float g, const float b, const float a)
//{
//	vertices[idx] = x1;
//	vertices[idx + 1] = y1;
//	vertices[idx + 2] = r;
//	vertices[idx + 3] = g;
//	vertices[idx + 4] = b;
//	vertices[idx + 5] = a;
//	vertices[idx + 6] = x2;
//	vertices[idx + 7] = y2;
//	vertices[idx + 8] = r;
//	vertices[idx + 9] = g;
//	vertices[idx + 10] = b;
//	vertices[idx + 11] = a;
//	idx += 12;
//}
//
//Gear::Core::Gear2D::PrimitiveBatch::CircleRenderer::CircleRenderer() :
//	vertices(new float[7 * maxCircleNum]), idx(0),
//	vertexBuffer(ResourceManager::createStructuredBufferView(sizeof(float) * 7, sizeof(float) * 7 * maxCircleNum, false, false, true, true, true))
//{
//}
//
//Gear::Core::Gear2D::PrimitiveBatch::CircleRenderer::~CircleRenderer()
//{
//	delete[] vertices;
//	delete vertexBuffer;
//}
//
//void Gear::Core::Gear2D::PrimitiveBatch::CircleRenderer::begin()
//{
//	idx = 0;
//}
//
//void Gear::Core::Gear2D::PrimitiveBatch::CircleRenderer::end(GraphicsContext* const context, ID3D12PipelineState* const pipelineState)
//{
//	if (idx > 0)
//	{
//		context->updateBuffer(vertexBuffer, vertices, sizeof(float) * idx);
//
//		context->setPipelineState(pipelineState);
//
//		context->setPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
//
//		context->setVertexBuffers(0, { vertexBuffer->getVertexBuffer() });
//
//		context->flushResourceBarriers();
//
//		context->draw(128, idx / 7, 0, 0);
//	}
//}
//
//void Gear::Core::Gear2D::PrimitiveBatch::CircleRenderer::addCircle(const float x, const float y, const float length, const float r, const float g, const float b, const float a)
//{
//	if (length < 1.f)
//	{
//		return;
//	}
//
//	vertices[idx] = x;
//	vertices[idx + 1] = y;
//	vertices[idx + 2] = length;
//	vertices[idx + 3] = r;
//	vertices[idx + 4] = g;
//	vertices[idx + 5] = b;
//	vertices[idx + 6] = a;
//	idx += 7;
//}
//
//Gear::Core::Gear2D::PrimitiveBatch::RCLineRenderer::RCLineRenderer() :
//	vertices(new float[7 * 2 * maxLineNum]), idx(0),
//	vertexBuffer(ResourceManager::createStructuredBufferView(sizeof(float) * 7, sizeof(float) * 7 * 2 * maxLineNum, false, false, true, true, true))
//{
//}
//
//Gear::Core::Gear2D::PrimitiveBatch::RCLineRenderer::~RCLineRenderer()
//{
//	delete[] vertices;
//	delete vertexBuffer;
//}
//
//void Gear::Core::Gear2D::PrimitiveBatch::RCLineRenderer::begin()
//{
//	idx = 0;
//}
//
//void Gear::Core::Gear2D::PrimitiveBatch::RCLineRenderer::end(GraphicsContext* const context, ID3D12PipelineState* const pipelineState)
//{
//	if (idx > 0)
//	{
//		context->updateBuffer(vertexBuffer, vertices, sizeof(float) * idx);
//
//		context->setPipelineState(pipelineState);
//
//		context->setPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
//
//		context->setVertexBuffers(0, { vertexBuffer->getVertexBuffer() });
//
//		context->flushResourceBarriers();
//
//		context->draw(idx / 7, 1, 0, 0);
//	}
//}
//
//void Gear::Core::Gear2D::PrimitiveBatch::RCLineRenderer::addRoundCapLine(const float x1, const float y1, const float x2, const float y2, const float width, const float r, const float g, const float b, const float a)
//{
//	vertices[idx] = x1;
//	vertices[idx + 1] = y1;
//	vertices[idx + 2] = width / 2.f;
//	vertices[idx + 3] = r;
//	vertices[idx + 4] = g;
//	vertices[idx + 5] = b;
//	vertices[idx + 6] = a;
//	vertices[idx + 7] = x2;
//	vertices[idx + 8] = y2;
//	vertices[idx + 9] = width / 2.f;
//	vertices[idx + 10] = r;
//	vertices[idx + 11] = g;
//	vertices[idx + 12] = b;
//	vertices[idx + 13] = a;
//
//	idx += 14;
//}
