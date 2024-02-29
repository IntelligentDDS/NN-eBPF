import torch.nn.functional as F
import torch.nn as nn
import torch

class Net(nn.Module):
    def __init__(self, in_dim, out_dim):
        super().__init__()
        self.model = nn.Sequential(
            nn.Linear(in_dim, 32, bias=False),
            nn.ReLU(),
            nn.Linear(32, 32, bias=False),
            nn.ReLU(),
            nn.Linear(32, out_dim, bias=False),
        )
    
    def forward(self, X):
        return self.model(X)
    
def train(num_epoch, train_loader, model, device, lr, model_path = None):
    criterion = nn.CrossEntropyLoss()
    optimizer = torch.optim.Adam(model.parameters(), lr=lr)
    model.to(device)
    for epoch in range(num_epoch):
        totoal_loss = 0
        for X, y in train_loader:
            X, y = X.to(device), y.to(device)
            output = model(X)
            loss = criterion(output, y)
            totoal_loss += loss.item()
            optimizer.zero_grad()
            loss.backward()
            optimizer.step()
        print(f'{epoch}: {totoal_loss / len(train_loader)}')
        if model_path is not None:
            torch.save(model, model_path)
    return model

def test(test_loader, model, device):
    model.to(device)
    criterion = nn.CrossEntropyLoss()
    with torch.no_grad():
        TP = 0
        FP = 0
        TN = 0
        FN = 0
        loss = 0
        for X, y in test_loader:
            X, y = X.to(device), y.to(device)
            import time
            # device = 'cpu'

            # start = time.time()
            output = model(X)
            loss += criterion(output, y).item()
            y_pred = F.softmax(output, dim=1).argmax(dim=1)
            TP += torch.logical_and(y, y_pred).sum().item()
            FP += torch.logical_and(torch.logical_not(y), y_pred).sum().item()
            TN += torch.logical_and(torch.logical_not(y), torch.logical_not(y_pred)).sum().item()
            FN += torch.logical_and(y, torch.logical_not(y_pred)).sum().item()
            # duration = time.time() - start
            # print(y_pred.shape)
        A = 0.0
        P = 0.0
        R = 0.0
        F2 = 0.0
        try:
            A = (TP + TN) / (TP + FP + TN + FN)
            P = TP / (TP + FP)
            R = TP / (TP + FN)
            F2 = 2 * P * R / (P + R)
            print(f'A:{A:.3f} P:{P:.3f} R:{R:.3f} F:{F2:.3f}')
        except ZeroDivisionError:
            print(TP, FP, FN)
        return (loss / len(test_loader), P, R, F2)