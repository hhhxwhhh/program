import torch
import torch.nn as nn
import torch.optim as optim
import time

def test_neural_network():
    """测试神经网络训练"""
    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
    print(f"使用设备: {device}")
    
    # 创建简单的神经网络
    class SimpleNet(nn.Module):
        def __init__(self):
            super(SimpleNet, self).__init__()
            self.fc1 = nn.Linear(784, 512)
            self.fc2 = nn.Linear(512, 256)
            self.fc3 = nn.Linear(256, 10)
            self.relu = nn.ReLU()
            
        def forward(self, x):
            x = self.relu(self.fc1(x))
            x = self.relu(self.fc2(x))
            x = self.fc3(x)
            return x
    
    # 创建模型和数据
    model = SimpleNet().to(device)
    criterion = nn.CrossEntropyLoss()
    optimizer = optim.Adam(model.parameters(), lr=0.001)
    
    # 模拟训练数据
    batch_size = 64
    num_batches = 100
    
    print("开始训练测试...")
    start_time = time.time()
    
    for batch_idx in range(num_batches):
        # 生成随机数据
        data = torch.randn(batch_size, 784).to(device)
        targets = torch.randint(0, 10, (batch_size,)).to(device)
        
        # 前向传播
        optimizer.zero_grad()
        outputs = model(data)
        loss = criterion(outputs, targets)
        
        # 反向传播
        loss.backward()
        optimizer.step()
        
        if batch_idx % 20 == 0:
            print(f"Batch {batch_idx}, Loss: {loss.item():.4f}")
    
    end_time = time.time()
    print(f"训练完成，总时间: {end_time - start_time:.2f}秒")
    
    # 显存使用情况
    if torch.cuda.is_available():
        print(f"显存使用: {torch.cuda.memory_allocated() // 1024**2} MB")
        print(f"显存缓存: {torch.cuda.memory_reserved() // 1024**2} MB")

if __name__ == "__main__":
    test_neural_network()