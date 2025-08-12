import pandas as pd
import numpy as np

# Define the SCS (Subcarrier Spacing) values in kHz
SCS_values = [15, 30, 60]

# Define the bandwidth values in MHz
bandwidth_values = [5, 10, 15, 20, 25, 30, 40, 50, 60, 70, 80, 90, 100]

# Define the data values from the table
data = np.array([
    [25, 52, 79, 106, 133, 160, 216, 270, np.nan, np.nan, np.nan, np.nan, np.nan],
    [11, 24, 38, 51, 65, 78, 106, 133, 162, 189, 217, 245, 273],
    [np.nan, 11, 18, 24, 31, 38, 51, 65, 79, 93, 107, 121, 135]
])

# Create a Pandas DataFrame
df_nprb = pd.DataFrame(data, index=SCS_values, columns=bandwidth_values)

# Rename the columns and index
df_nprb.columns.name = 'Bandwidth (MHz)'
df_nprb.index.name = 'SCS (kHz)'

# Define the algorithms along with their IQ width and scaler number
algorithm_data = {
    "BFP9": {"IQ_Width": 9, "Scaler_Num": 8},
    "BS": {"IQ_Width": 8, "Scaler_Num": 16},
    "uLaw": {"IQ_Width": 6, "Scaler_Num": 4},
    "M8": {"IQ_Width": 8, "Scaler_Num": 0},
    "M4": {"IQ_Width": 4, "Scaler_Num": 0}
}

# Create a Pandas DataFrame from the dictionary
df_algorithm = pd.DataFrame.from_dict(algorithm_data, orient='index')

def traffic_def(bw, scs, tdd, symb, cmp_algthm, n_port, n_tx):
    iq_width = df_algorithm.loc[cmp_algthm, "IQ_Width"]
    scaler_num = df_algorithm.loc[cmp_algthm, "Scaler_Num"]
    nprb = df_nprb.loc[scs, bw]
    user_rb = tdd * ((((((2 * iq_width * 12 + scaler_num) * nprb) + 36*8)* symb * n_port)) / (1/ ((scs / 15)) * (10 ** (-3))))/1e9
    user_pkt_size = ((2 * iq_width * 12 + scaler_num)/8)  * nprb + 36
    control_pkt_size = ((2 * iq_width)* nprb) / 8
    control_rb = ((2 * iq_width * nprb) * n_port*n_tx) / (1/ ((scs / 15)) * (10 ** (-3)))/1e9
    print(f"PRB: {nprb} || U-Rb {user_rb} Gbps || U-Pkt {user_pkt_size} B || C-Rb {control_rb} Gbps || c-Pkt {control_pkt_size} B || SlotTime: {(1 / ((scs / 15)))} ms")
    return user_pkt_size, user_rb, control_pkt_size, control_rb
def main():
    
    
    
    traffic_def(100, 30, 1, 14, "BFP9", 4, 4)

if __name__ == '__main__':
    main()
