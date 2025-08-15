import pandas as pd

# Read the CSV file
df = pd.read_csv('results/stats_100.csv')

# Filter for len == 65536
df_filtered = df[df['len'] == 65536]

# Further filter for:
# - PROC=32 for MS and FD models
# - PROC=1 for SQ models
df_filtered = pd.concat([
    df_filtered[(df_filtered['model'].isin(['MS4', 'FD4', 'MS2', 'FD2'])) & (df_filtered['proc'] == 32)],
    df_filtered[(df_filtered['model'].isin(['SQ4', 'SQ2'])) & (df_filtered['proc'] == 1)]
])

# Extract baseline SQ times for each group (determined by the last character, e.g., '4' or '2')
baseline = {}
for model in df_filtered['model'].unique():
    if model.startswith('SQ'):
        group = model[-1]  # e.g., '4' or '2'
        baseline[group] = df_filtered.loc[df_filtered['model'] == model, 'time'].iloc[0]

# Function to calculate the speed-up for each row
def calculate_speedup(row):
    if row['model'].startswith('SQ'):
        return 1
    else:
        group = row['model'][-1]  # Extract group identifier
        return baseline[group] / row['time']

# Apply the function to create the "speed-up" column
df_filtered['speed-up'] = df_filtered.apply(calculate_speedup, axis=1)

# Drop the 'len' and 'itr' columns
df_filtered = df_filtered.drop(columns=['len', 'itr'])

# Order rows as SQ4, SQ2, MS4, MS2, FD4, FD2
order = ['SQ4', 'MS4', 'FD4', 'SQ2', 'MS2', 'FD2']
df_filtered['model'] = pd.Categorical(df_filtered['model'], categories=order, ordered=True)
df_filtered = df_filtered.sort_values('model')

# Rename columns to match the custom header:
# "Model & Processors & Execution Time (s) & Speed-up"
df_filtered = df_filtered.rename(columns={
    'model': 'Model',
    'proc': 'Processors',
    'time': 'Execution Time (s)',
    'speed-up': 'Speed-up'
})

# Define formatters:
# - Format "Execution Time (s)" to one decimal place.
# - Format "Speed-up" to one decimal place and add the $\times$ symbol.
formatters = {
    'Execution Time (s)': lambda x: '%.1f' % x,
    'Speed-up': lambda x: '%.1f$\\times$' % x
}

# Convert the DataFrame to LaTeX format with escape disabled, and print
latex_output = df_filtered.to_latex(index=False, formatters=formatters, escape=False)
print(latex_output)
