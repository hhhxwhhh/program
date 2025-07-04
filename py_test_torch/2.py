import torch
import matplotlib.pyplot as plt
import numpy as np

# 获取PyTorch版本
pytorch_version = torch.__version__
print(f"PyTorch版本: {pytorch_version}")

# 检查CUDA是否可用
cuda_available = torch.cuda.is_available()
print(f"CUDA可用: {cuda_available}")

# 如果CUDA可用，显示GPU信息
if cuda_available:
    print(f"CUDA版本: {torch.version.cuda}")
    print(f"GPU数量: {torch.cuda.device_count()}")
    
    for i in range(torch.cuda.device_count()):
        # 获取内存信息（以MB为单位）
        memory_bytes = torch.cuda.get_device_properties(i).total_memory
        memory_mb = memory_bytes // (1024 * 1024)
        print(f"GPU {i} 总内存: {memory_mb} MB")
        
        # 获取GPU名称
        device_name = torch.cuda.get_device_name(i)
        print(f"GPU {i} 名称: {device_name}")
        
        # 获取当前显存使用情况
        allocated = torch.cuda.memory_allocated(i) // (1024 * 1024)
        cached = torch.cuda.memory_reserved(i) // (1024 * 1024)
        print(f"GPU {i} 已分配内存: {allocated} MB")
        print(f"GPU {i} 缓存内存: {cached} MB")
        print("-" * 50)
else:
    print("CUDA not available")
    print("No GPU detected")