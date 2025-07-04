import torch
import torchvision
import numpy as np
#import matplotlib.pyplot as plt  # 添加这行
from PIL import Image
import os
import random
import time
import torch.nn as nn

def check_cuda_detailed():
    """详细检查CUDA配置"""
    print("=== Detailed CUDA Check ===")
    
    # 基本CUDA可用性
    cuda_available = torch.cuda.is_available()
    print(f"CUDA available: {cuda_available}")
    
    if cuda_available:
        # CUDA版本信息
        print(f"CUDA version (PyTorch compiled with): {torch.version.cuda}")
        print(f"cuDNN version: {torch.backends.cudnn.version()}")
        print(f"cuDNN enabled: {torch.backends.cudnn.enabled}")
        
        # GPU设备信息
        gpu_count = torch.cuda.device_count()
        print(f"GPU count: {gpu_count}")
        
        for i in range(gpu_count):
            print(f"\n--- GPU {i} Details ---")
            print(f"  Name: {torch.cuda.get_device_name(i)}")
            
            # GPU内存信息
            memory_total = torch.cuda.get_device_properties(i).total_memory / 1024**3
            memory_reserved = torch.cuda.memory_reserved(i) / 1024**3
            memory_allocated = torch.cuda.memory_allocated(i) / 1024**3
            
            print(f"  Total memory: {memory_total:.2f} GB")
            print(f"  Reserved memory: {memory_reserved:.2f} GB")
            print(f"  Allocated memory: {memory_allocated:.2f} GB")
            print(f"  Available memory: {memory_total - memory_reserved:.2f} GB")
            
            # GPU属性
            props = torch.cuda.get_device_properties(i)
            print(f"  Compute capability: {props.major}.{props.minor}")
            print(f"  Multi-processor count: {props.multi_processor_count}")
        
        # 当前设备
        current_device = torch.cuda.current_device()
        print(f"\nCurrent CUDA device: {current_device}")
        
        # CUDA张量测试
        print("\n--- CUDA Functionality Test ---")
        try:
            # 在GPU上创建张量
            gpu_tensor = torch.randn(3, 3).cuda()
            print(f"✅ GPU tensor creation successful: {gpu_tensor.device}")
            
            # GPU计算测试
            gpu_result = gpu_tensor @ gpu_tensor.T
            print(f"✅ GPU matrix multiplication successful")
            
            # CPU到GPU数据传输测试
            cpu_tensor = torch.randn(3, 3)
            gpu_from_cpu = cpu_tensor.to('cuda')
            print(f"✅ CPU to GPU transfer successful")
            
            # GPU到CPU数据传输测试
            cpu_from_gpu = gpu_tensor.cpu()
            print(f"✅ GPU to CPU transfer successful")
            
        except Exception as e:
            print(f"❌ CUDA functionality test failed: {e}")
            
    else:
        print("❌ CUDA not available. Possible reasons:")
        print("  1. CUDA not installed on system")
        print("  2. PyTorch installed without CUDA support")
        print("  3. No compatible GPU found")
        print("  4. CUDA driver issues")
        
        # 检查是否安装了CPU版本的PyTorch
        print(f"\nPyTorch installation info:")
        print(f"  Version: {torch.__version__}")
        if "+cpu" in torch.__version__:
            print("  ⚠️  You have CPU-only PyTorch installed")
            print("  To install CUDA version, visit: https://pytorch.org/")

def check_system_cuda():
    """检查系统级CUDA安装"""
    print("\n=== System CUDA Check ===")
    
    import subprocess
    import sys
    
    # 检查nvidia-smi
    try:
        result = subprocess.run(['nvidia-smi'], capture_output=True, text=True, timeout=10)
        if result.returncode == 0:
            print("✅ nvidia-smi available")
            # 提取CUDA版本信息
            lines = result.stdout.split('\n')
            for line in lines:
                if 'CUDA Version:' in line:
                    cuda_version = line.split('CUDA Version:')[1].strip().split()[0]
                    print(f"System CUDA version: {cuda_version}")
                    break
        else:
            print("❌ nvidia-smi not found or failed")
    except (subprocess.TimeoutExpired, FileNotFoundError, subprocess.SubprocessError):
        print("❌ nvidia-smi not available")
    
    # 检查nvcc
    try:
        result = subprocess.run(['nvcc', '--version'], capture_output=True, text=True, timeout=10)
        if result.returncode == 0:
            print("✅ nvcc (CUDA compiler) available")
            # 提取编译器版本
            for line in result.stdout.split('\n'):
                if 'release' in line.lower():
                    print(f"NVCC version info: {line.strip()}")
                    break
        else:
            print("❌ nvcc not found")
    except (subprocess.TimeoutExpired, FileNotFoundError, subprocess.SubprocessError):
        print("❌ nvcc not available")

def print_library_info():
    print("=== Deep Learning Environment Check ===")
    print(f"✅ All libraries imported successfully!")
    print()
    
    # PyTorch信息
    print("=== PyTorch Information ===")
    print(f"PyTorch version: {torch.__version__}")
    print(f"Torchvision version: {torchvision.__version__}")
    
    # 详细CUDA检查
    check_cuda_detailed()
    
    print()
    
    # NumPy信息
    print("=== NumPy Information ===")
    print(f"NumPy version: {np.__version__}")
    print()
    
    # PIL信息
    print("=== PIL Information ===")
    print(f"Pillow version: {Image.__version__}")
    print()
    
    # 系统信息
    print("=== System Information ===")
    print(f"Python executable: {os.sys.executable}")
    print(f"Platform: {os.sys.platform}")
    
    # 系统级CUDA检查
    check_system_cuda()
    
    print()
    
    # 简单功能测试
    print("=== Quick Functionality Test ===")
    
    # PyTorch张量测试
    test_tensor = torch.randn(3, 3)
    print(f"✅ PyTorch tensor creation: {test_tensor.shape}")
    
    # NumPy数组测试
    test_array = np.random.random((3, 3))
    print(f"✅ NumPy array creation: {test_array.shape}")
    
    # 简单神经网络层测试
    test_layer = nn.Linear(10, 5)
    test_input = torch.randn(1, 10)
    test_output = test_layer(test_input)
    print(f"✅ Neural network layer test: {test_output.shape}")
    
    print()
    
    if torch.cuda.is_available():
        print("🎉 Your environment is ready for GPU-accelerated deep learning!")
    else:
        print("📝 Your environment is ready for CPU-based deep learning!")
        print("💡 For GPU acceleration, ensure CUDA is properly installed.")

if __name__ == "__main__":
    print_library_info()