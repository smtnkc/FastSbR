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
    time_cols = [f'time_{i}' for i in range(1, len(dataframes) + 1)]
    itr_cols = [f'itr_{i}' for i in range(1, len(dataframes) + 1)]
    df['time'] = df[time_cols].mean(axis=1)
    df['itr'] = df[itr_cols].mean(axis=1)
else:
    df = dataframes[0]  # If only one file, use it directly

# Create a 2x2 subplot layout
fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(12, 10))
font_size = 14  # Define font size variable
plt.rcParams.update({'font.size': font_size})  # Set global font size
models = ['MS2', 'MS4', 'FD2', 'FD4']
display_names = ['MW2', 'MW4', 'FD2', 'FD4']  # Display names for the plots
axes = [ax1, ax2, ax3, ax4]
references = ['SQ2', 'SQ4', 'SQ2', 'SQ4']

# Plot data for each model
for model, display_name, ref_model, ax in zip(models, display_names, references, axes):
    # Plot main model data
    model_data = df[df['model'] == model]
    for proc in sorted(model_data['proc'].unique()):
        data = model_data[model_data['proc'] == proc]
        ax.plot(data['len'], data['time'], marker='o', label=f'{display_name} ({proc} proc)')

    # Plot reference model (SQ2/SQ4)
    ref_data = df[(df['model'] == ref_model) & (df['proc'] == 1)]
    linestyle = ':' if ref_model == 'SQ2' else '--'  # Use dotted line for SQ2, dashed for SQ4
    ax.plot(ref_data['len'], ref_data['time'], marker='s', linestyle=linestyle, 
           color='black', label=f'{ref_model}')

    ax.set_xlabel('Length', fontsize=font_size)
    ax.set_ylabel('Time (seconds)', fontsize=font_size)
    ax.set_title(f'{display_name} vs {ref_model} Performance', fontsize=font_size)
    ax.tick_params(axis='both', which='major', labelsize=font_size-2)  # Set tick label size to font_size-2
    ax.grid(True)
    ax.legend(fontsize=font_size)

# Adjust layout to prevent overlap
plt.tight_layout()
plt.savefig('results/Figure_1.pdf')
plt.show()
