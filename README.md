# 介绍

为了学习图形学写的一个D3D12图形引擎

## 关于如何创建新项目

直接基于SampleProject创建模板项目即可

## 打开日志

1. 打开Windows Terminal
2. chcp 65001
3. type log.txt

## 待完成事项

<!-- TODO Begin -->
- 优化RootSignature类，目前根参数索引都是硬编码的，真得好好重写一下
- RAII 引擎目前大量使用了裸指针、new、delete。这对于以后的开发来说是大隐患
<!-- TODO END -->