import torch
import os

print("=" * 50)
print("PyTorch详细信息:")
print("=" * 50)
print(f"PyTorch版本: {torch.__version__}")
print(f"是否为CUDA版本: {'+cu' in torch.__version__}")
print(f"CUDA可用: {torch.cuda.is_available()}")
print(f"CUDA设备数量: {torch.cuda.device_count()}")
try:
    print(f"当前CUDA设备: {torch.cuda.current_device()}")
except:
    print("当前CUDA设备: 无可用设备")

if hasattr(torch.version, 'cuda') and torch.version.cuda:
    print(f"PyTorch编译时的CUDA版本: {torch.version.cuda}")
else:
    print("PyTorch编译时的CUDA版本: None (CPU版本)")

print(f"系统CUDA版本: 12.3 (从nvidia-smi)")
print("=" * 50)

# 检查numpy兼容性
print("NumPy兼容性检查:")
print("=" * 50)
try:
    import numpy as np
    print(f"NumPy版本: {np.__version__}")
    
    # 测试torch和numpy之间的转换
    print("测试torch-numpy转换:")
    
    # 创建torch张量
    torch_tensor = torch.randn(3, 3)
    print(f"原始torch张量设备: {torch_tensor.device}")
    
    # 转换为numpy
    numpy_array = torch_tensor.numpy()
    print(f"转换为numpy成功: {type(numpy_array)}")
    print(f"numpy数组形状: {numpy_array.shape}")
    
    # 从numpy创建torch张量
    torch_from_numpy = torch.from_numpy(numpy_array)
    print(f"从numpy创建torch张量成功: {torch_from_numpy.device}")
    
    # 检查数据是否一致
    is_equal = torch.allclose(torch_tensor, torch_from_numpy)
    print(f"数据一致性检查: {is_equal}")
    
    print("✅ Torch-NumPy转换正常")
    
except Exception as e:
    print(f"❌ Torch-NumPy转换错误: {e}")

print("=" * 50)

# 检查安装的具体包
print("包版本信息:")
print("=" * 50)
try:
    import pkg_resources
    
    # 检查相关包版本
    packages = ['torch', 'numpy', 'torchvision', 'torchaudio']
    for pkg_name in packages:
        try:
            pkg = pkg_resources.get_distribution(pkg_name)
            print(f"{pkg_name}: {pkg.version}")
        except pkg_resources.DistributionNotFound:
            print(f"{pkg_name}: 未安装")
            
except Exception as e:
    print(f"无法获取包信息: {e}")

print("=" * 50)

# 检查CUDA相关环境变量
print("CUDA环境变量:")
print("=" * 50)
cuda_vars = ['CUDA_PATH', 'CUDA_HOME', 'CUDA_ROOT', 'PATH']
for var in cuda_vars:
    value = os.environ.get(var)
    if var == 'PATH':
        # 只显示包含CUDA的PATH部分
        if value:
            cuda_paths = [p for p in value.split(';') if 'cuda' in p.lower()]
            if cuda_paths:
                print(f"{var} (CUDA相关): {cuda_paths}")
            else:
                print(f"{var}: 无CUDA相关路径")
    else:
        print(f"{var}: {value if value else '未设置'}")

print("=" * 50)

# 尝试检测CUDA库
print("CUDA库检测:")
print("=" * 50)
try:
    # 检查是否能加载CUDA运行时
    torch.cuda.init()
    print("CUDA运行时初始化: 成功")
except Exception as e:
    print(f"CUDA运行时初始化: 失败 - {e}")

try:
    # 检查cuDNN
    if torch.cuda.is_available():
        print(f"cuDNN可用: {torch.backends.cudnn.enabled}")
        print(f"cuDNN版本: {torch.backends.cudnn.version()}")
    else:
        print("cuDNN: CUDA不可用，无法检查cuDNN")
except Exception as e:
    print(f"cuDNN检查失败: {e}")

print("=" * 50)

# 内存诊断
print("内存诊断:")
print("=" * 50)
try:
    # CPU内存
    import psutil
    cpu_memory = psutil.virtual_memory()
    print(f"系统内存: {cpu_memory.total // (1024**3)} GB")
    print(f"可用内存: {cpu_memory.available // (1024**3)} GB")
except ImportError:
    print("未安装psutil，无法检查系统内存")

if torch.cuda.is_available():
    try:
        for i in range(torch.cuda.device_count()):
            props = torch.cuda.get_device_properties(i)
            print(f"GPU {i} 总内存: {props.total_memory // (1024**2)} MB")
            print(f"GPU {i} 已用内存: {torch.cuda.memory_allocated(i) // (1024**2)} MB")
    except Exception as e:
        print(f"GPU内存检查失败: {e}")
else:
    print("GPU内存: CUDA不可用")

print("=" * 50)
print("诊断完成")