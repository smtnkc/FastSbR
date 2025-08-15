import pandas as pd
import matplotlib.pyplot as plt

# Read CSV files from a list
file_list = ['results/stats_100.csv', 'results/stats_101.csv', 'results/stats_102.csv', 'results/stats_103.csv', 'results/stats_104.csv',
             'results/stats_105.csv','results/stats_106.csv', 'results/stats_107.csv', 'results/stats_108.csv', 'results/stats_109.csv']
dataframes = [pd.read_csv(file, sep=',') for file in file_list]

# Merge dataframes if there's more than one file
if len(dataframes) > 1:
    df = dataframes[0]
    # Keep the first dataframe's 'time' column as is
    df = df.rename(columns={'time': 'time_1', 'itr': 'itr_1'})
    
    for i, next_df in enumerate(dataframes[1:], 2):  # Start from 2
        df = pd.merge(df, next_df, on=['len', 'proc', 'model'])
        # Rename columns to avoid conflicts
        df = df.rename(columns={
            'time': f'time_{i}',
            'itr': f'itr_{i}'
        })
    
    # Calculate mean values across all files
    time_cols = [f'time_{i}' for i in range(1, len(dataframes) + 1)]  # Start from 1
    itr_cols = [f'itr_{i}' for i in range(1, len(dataframes) + 1)]    # Start from 1
    df['time'] = df[time_cols].mean(axis=1)
    df['itr'] = df[itr_cols].mean(axis=1)
else:
    df = dataframes[0]  # If only one file, use it directly

# Create the plot
plt.figure(figsize=(7, 5))

# Plot sequential methods
sq2_data = df[(df['model'] == 'SQ2') & (df['proc'] == 1)]
plt.plot(sq2_data['len'], sq2_data['time'], marker='s', linestyle=':', 
         color='black', label='SQ2')

sq4_data = df[(df['model'] == 'SQ4') & (df['proc'] == 1)]
plt.plot(sq4_data['len'], sq4_data['time'], marker='s', linestyle='--', 
         color='black', label='SQ4')


n_proc = 32

# Plot 2-approx methods (32 proc)

ms2_data = df[(df['model'] == 'MS2') & (df['proc'] == n_proc)]
plt.plot(ms2_data['len'], ms2_data['time'], marker='o', 
         color='green', label=f'MW2 ({n_proc} proc)')

fd2_data = df[(df['model'] == 'FD2') & (df['proc'] == n_proc)]
plt.plot(fd2_data['len'], fd2_data['time'], marker='^', 
         color='orange', label=f'FD2 ({n_proc} proc)')


# Plot 4-approx methods (32 proc)
ms4_data = df[(df['model'] == 'MS4') & (df['proc'] == n_proc)]
plt.plot(ms4_data['len'], ms4_data['time'], marker='o', 
         color='red', label=f'MW4 ({n_proc} proc)')

fd4_data = df[(df['model'] == 'FD4') & (df['proc'] == n_proc)]
plt.plot(fd4_data['len'], fd4_data['time'], marker='^', 
         color='blue', label=f'FD4 ({n_proc} proc)')

# Define font sizes
font_size = 14
tick_font_size = font_size - 2

plt.xlabel('Length', fontsize=font_size)
plt.ylabel('Time (seconds)', fontsize=font_size)
plt.title('Performance Comparison of All Methods', fontsize=font_size)
plt.grid(True)
plt.legend(fontsize=font_size)

# Set tick font sizes
plt.xticks(fontsize=tick_font_size)
plt.yticks(fontsize=tick_font_size)

plt.tight_layout()
plt.savefig('results/Figure_2.pdf')
plt.show()
