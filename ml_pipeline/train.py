import torch
import torch.nn as nn
import torch.optim as optim
import struct
import numpy as np

# 1. Define the exact same architecture as our C++ engine
# 768 inputs (64 squares * 12 pieces) -> 256 Hidden Neurons -> 1 Output Score
class ChessNNUE(nn.Module):
    def __init__(self):
        super(ChessNNUE, self).__init__()
        self.fc1 = nn.Linear(768, 256)
        self.relu = nn.ReLU()
        self.fc2 = nn.Linear(256, 1)

    def forward(self, x):
        x = self.fc1(x)
        x = self.relu(x)
        x = self.fc2(x)
        return x

def generate_dummy_data(num_samples=1000):
    print(f"Generating {num_samples} dummy board positions...")
    # Random 1s and 0s representing pieces on a board
    X = torch.randint(0, 2, (num_samples, 768)).float()
    
    # Random centipawn evaluations (-1000 to +1000)
    Y = torch.randint(-1000, 1000, (num_samples, 1)).float()
    
    return X, Y

def train_model():
    model = ChessNNUE()
    optimizer = optim.Adam(model.parameters(), lr=0.001)
    loss_fn = nn.MSELoss() # Mean Squared Error is best for centipawns

    X_train, Y_train = generate_dummy_data(10000)

    print("Starting Training Loop...")
    epochs = 50
    for epoch in range(epochs):
        optimizer.zero_grad()
        
        # Forward pass
        predictions = model(X_train)
        
        # Calculate loss (how wrong the AI is compared to Stockfish)
        loss = loss_fn(predictions, Y_train)
        
        # Backpropagation (updating the weights to be smarter)
        loss.backward()
        optimizer.step()
        
        if epoch % 10 == 0:
            print(f"Epoch {epoch}/{epochs} | Loss: {loss.item():.2f}")

    print("Training Complete!")
    return model

def export_to_bin(model, filename="network.bin"):
    print(f"Exporting weights to {filename} for C++ Engine...")
    
    # Extract weights from PyTorch
    w1 = model.fc1.weight.detach().numpy() # Shape: [256, 768]
    b1 = model.fc1.bias.detach().numpy()   # Shape: [256]
    w2 = model.fc2.weight.detach().numpy() # Shape: [1, 256]
    b2 = model.fc2.bias.detach().numpy()   # Shape: [1]

    # Our C++ expects weights_input_hidden as [768][256], so we MUST transpose!
    w1_transposed = w1.T # Now Shape: [768, 256]
    
    with open(filename, "wb") as f:
        # 1. Write Layer 1 Weights (768 * 256 floats)
        f.write(w1_transposed.astype(np.float32).tobytes())
        
        # 2. Write Layer 1 Biases (256 floats)
        f.write(b1.astype(np.float32).tobytes())
        
        # 3. Write Layer 2 Weights (256 floats)
        # Note: w2 is [1, 256], flatten it to just 256 elements
        f.write(w2.flatten().astype(np.float32).tobytes())
        
        # 4. Write Layer 2 Bias (1 float)
        f.write(b2.astype(np.float32).tobytes())

    print(f"Success! Move {filename} into your C++ engine directory.")

if __name__ == "__main__":
    trained_model = train_model()
    export_to_bin(trained_model, "network.bin")