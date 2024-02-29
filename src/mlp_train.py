from sklearn import preprocessing
from sklearn import model_selection
import numpy as np
from mlp import Net, train, test
import torch.utils.data as Data
import torch

data_path = '../dataset/reproduction-xdp-data.npy'
label_path = '../dataset/reproduction-xdp-label.npy'
model_path = '../dataset/reproduction-xdp-MLP.pkl'

batch_size = 512
learning_rate = 1e-3
num_epoch = 32
device = 'cuda' if torch.cuda.is_available() else 'cpu'

data = np.load(data_path)
label = np.load(label_path, allow_pickle=True)

# 划分训练集和测试集
X_train, X_test, y_train, y_test = model_selection.train_test_split(
    data, label, train_size=0.8, test_size=0.2, stratify=label)
print("X_train, y_train:", X_train.shape, y_train.shape)
print("X_test, y_test:", X_test.shape, y_test.shape)

# 保存所用
save_X_test = X_test.copy()

# 数据标准化
standard_scaler = preprocessing.StandardScaler()
standard_scaler.fit(data)
X_train = standard_scaler.transform(X_train)
X_test = standard_scaler.transform(X_test)

# 用pytorch进行训练和测试
train_loader = Data.DataLoader(
    dataset=Data.TensorDataset(torch.from_numpy(X_train).float(), torch.from_numpy(y_train).long()),
    batch_size=batch_size,
    shuffle=True,
    num_workers=4,
)

test_loader = Data.DataLoader(
    dataset=Data.TensorDataset(torch.from_numpy(X_test).float(), torch.from_numpy(y_test).long()),
    batch_size=batch_size,
    shuffle=True, 
    num_workers=4,
)

model = Net(X_train.shape[1], 2)
print('Training...')
model = train(num_epoch, train_loader, model, device, learning_rate, model_path)
# model = torch.load(model_path)
print('Testing...')
test(test_loader, model, device)

torch.save({'state_dict': model.state_dict(),
            'train_loader': train_loader,
            'X_test': X_test,
            'y_test': y_test,
            'raw_X_test': save_X_test,
            'mean': standard_scaler.mean_,
            'scale': standard_scaler.scale_},
            'mlp.th')
