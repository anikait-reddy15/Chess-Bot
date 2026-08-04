import os
import glob
import pandas as pd
import numpy as np
import torch
import torch.nn as nn
import torch.optim as optim
from torch.utils.data import Dataset, DataLoader

# HalfKP Architecture (40,960 -> 256 -> 1)
class ChessNNUE(nn.Module):
    def __init__(self):
        super(ChessNNUE, self).__init__()
        self.fc1 = nn.Linear(40960, 256) 
        self.relu = nn.ReLU()
        self.fc2 = nn.Linear(256, 1)

    def forward(self, x):
        x = self.fc1(x)
        x = self.relu(x)
        x = self.fc2(x)
        return x

def parse_fen_halfkp(fen, evaluation):
    parts = fen.split()
    board = parts[0]
    side_to_move = parts[1]

    features = np.zeros(40960, dtype=np.float32)
    king_char = 'K' if side_to_move == 'w' else 'k'
    king_sq = -1
    
    pieces_on_board = []
    square = 0
    
    # Locate all pieces and the King
    for char in board:
        if char == '/':
            continue
        elif char.isdigit():
            square += int(char)
        else:
            if char == king_char:
                king_sq = square
            pieces_on_board.append((char, square))
            square += 1
            
    if side_to_move == 'b':
        king_sq ^= 56
        
    def get_piece_id(char, side):
        is_white = char.isupper()
        c = char.lower()
        base_id = {'p':0, 'n':1, 'b':2, 'r':3, 'q':4}.get(c, -1)
        if base_id == -1: return -1 # Skip Kings
        
        is_friendly = (is_white and side == 'w') or (not is_white and side == 'b')
        return base_id if is_friendly else base_id + 5

    # Map the pieces relative to the King
    for char, sq in pieces_on_board:
        piece_id = get_piece_id(char, side_to_move)
        if piece_id == -1: continue 
        
        mapped_sq = sq ^ 56 if side_to_move == 'b' else sq
        feature_idx = (king_sq * 640) + (piece_id * 64) + mapped_sq
        features[feature_idx] = 1.0

    score = float(evaluation)
    if side_to_move == 'b':
        score = -score
        
    score = max(-2000.0, min(2000.0, score))
    return features, score

class ChessDataset(Dataset):
    def __init__(self, df):
        # We only store the strings in RAM to prevent an 80GB memory crash!
        if 'FEN' in df.columns and 'Evaluation' in df.columns:
            self.fens = df['FEN'].values
            self.evals = df['Evaluation'].values
        else:
            self.fens = df.iloc[:, 0].values
            self.evals = df.iloc[:, 1].values

    def __len__(self):
        return len(self.fens)

    def __getitem__(self, idx):
        # "Lazy Loading": We generate the 40,960 array ON THE FLY right when PyTorch asks for it
        ev = str(self.evals[idx])
        if '#' in ev:
            ev = 10000 if not '-' in ev else -10000
            
        features, score = parse_fen_halfkp(self.fens[idx], ev)
        return features, np.array([score], dtype=np.float32)

def export_to_bin(model, filename="network.bin"):
    print(f"\nExporting HalfKP weights to {filename} for C++ Engine...")
    
    w1 = model.fc1.weight.detach().numpy()
    b1 = model.fc1.bias.detach().numpy()
    w2 = model.fc2.weight.detach().numpy()
    b2 = model.fc2.bias.detach().numpy()

    w1_transposed = w1.T 
    
    with open(filename, "wb") as f:
        f.write(w1_transposed.astype(np.float32).tobytes())
        f.write(b1.astype(np.float32).tobytes())
        f.write(w2.flatten().astype(np.float32).tobytes())
        f.write(b2.astype(np.float32).tobytes())
        
    print(f"Export successful! Saved to: {filename}")

if __name__ == "__main__":
    project_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    data_dir = os.path.join(project_root, "data")
    csv_files = glob.glob(os.path.join(data_dir, "*.csv"))
    
    if not csv_files:
        print("ERROR: Could not find any .csv files in the 'data' folder.")
        exit(1)
        
    csv_path = csv_files[0] 
    
    CHUNK_SIZE = 500000  
    BATCH_SIZE = 1024     
    EPOCHS = 10           
    LEARNING_RATE = 0.001
    
    # Utilize GPU if available to speed up training
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    print(f"Training on device: {device}")
    
    model = ChessNNUE().to(device)
    optimizer = optim.Adam(model.parameters(), lr=LEARNING_RATE)
    loss_fn = nn.MSELoss()

    print(f"Starting HalfKP Chunked Training over {EPOCHS} Epochs...")
    
    for epoch in range(EPOCHS):
        print(f"\n========== EPOCH {epoch+1}/{EPOCHS} ==========")
        
        chunk_iterator = pd.read_csv(csv_path, chunksize=CHUNK_SIZE)
        epoch_loss = 0.0
        total_chunks = 0
        
        for chunk_idx, chunk_df in enumerate(chunk_iterator):
            print(f"  [Epoch {epoch+1}] Parsing Chunk {chunk_idx+1} ({len(chunk_df)} FENs)...")
            
            # This dataset now parses FENs dynamically to save RAM
            dataset = ChessDataset(chunk_df)
            
            # num_workers=4 will use multiple CPU cores to parse FENs faster (Windows users may need to set this to 0 if it causes issues)
            dataloader = DataLoader(dataset, batch_size=BATCH_SIZE, shuffle=True)
            
            chunk_loss = 0.0
            
            for batch_X, batch_Y in dataloader:
                batch_X, batch_Y = batch_X.to(device), batch_Y.to(device)
                
                optimizer.zero_grad()
                predictions = model(batch_X)
                loss = loss_fn(predictions, batch_Y)
                loss.backward()
                optimizer.step()
                
                chunk_loss += loss.item()
                
            avg_chunk_loss = chunk_loss / len(dataloader)
            print(f"  -> Chunk {chunk_idx+1} Avg Loss: {avg_chunk_loss:.2f}")
            
            epoch_loss += avg_chunk_loss
            total_chunks += 1
            
        print(f"*** End of Epoch {epoch+1} | Overall Avg Loss: {(epoch_loss/total_chunks):.2f} ***")

    # Save to the root folder so the C++ engine immediately finds it!
    output_path = os.path.join(project_root, "network.bin")
    
    # Move model back to CPU for exporting weights to numpy
    model.to("cpu")
    export_to_bin(model, output_path)