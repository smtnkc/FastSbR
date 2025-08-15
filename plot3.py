import pandas as pd
import matplotlib.pyplot as plt

# Read the CSV file with semicolon separator
df = pd.read_csv('results/stats_109.csv', sep=',')

# Create the plot
plt.figure(figsize=(7, 4))

# Plot sequential methods
sq4_data = df[(df['model'] == 'SQ4') & (df['proc'] == 1)]
sq2_data = df[(df['model'] == 'SQ2') & (df['proc'] == 1)]
plt.plot(sq4_data['len'], sq4_data['itr'], marker='s', linestyle='--', 
         color='black', label='SQ4')
plt.plot(sq2_data['len'], sq2_data['itr'], marker='s', fillstyle='none', linestyle=':', 
         color='gray', label='SQ2')
# Define font sizes
font_size = 14
tick_font_size = font_size - 2
# Remove other plot calls and directly add labels
plt.xlabel('Length', fontsize=font_size)
plt.ylabel('Number of Reversals', fontsize=font_size)
plt.title('Reversal Count Comparison', fontsize=font_size)
plt.grid(True)
plt.legend(fontsize=font_size)



# Set tick font sizes
plt.xticks(fontsize=tick_font_size)
plt.yticks(fontsize=tick_font_size)

plt.tight_layout()
plt.savefig('results/Figure_3.pdf')
plt.show()
