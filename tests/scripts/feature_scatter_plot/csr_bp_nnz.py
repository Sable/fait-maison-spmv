import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import sys

L1 = 32 * 1024
L2 = 256 * 1024
L3 = 12*1024*1024
MM = 16*1024*1024*1024
out_dir = '../../results/scatter_plots/'

df = pd.read_csv(sys.argv[1], index_col = 'name')
df = df.loc[df.nnz_per_row < 15]
df_xlarge = pd.DataFrame()
df_large = pd.DataFrame()
df_medium = pd.DataFrame()
df_small = pd.DataFrame()


df_coo = pd.read_csv(sys.argv[2], index_col = 'name')

for index, row in df.iterrows():
  if (int(row['N']) * 8) < MM and (int(row['N']) * 8) >= L3 :
    df_xlarge = df_xlarge.append(row)
  if (int(row['N']) * 8) < L3 and (int(row['N']) * 8) >= L2 :
    df_large = df_large.append(row)
  if (int(row['N']) * 8) < L2 and (int(row['N']) * 8) >= L1 :
    df_medium = df_medium.append(row)
  if (int(row['N']) * 8) < L1 :
    df_small = df_small.append(row)


plt.xlabel('avg_nnz_per_row', fontsize=15)
plt.ylabel('Performance (MFLOPS)', fontsize=15)
plt.xticks(fontsize=15)
plt.yticks(fontsize=15)
bp = np.concatenate([df['bp'], df_coo['bp']], axis = 0)
min_bp = bp.min()
max_bp = bp.max()
print min_bp
print max_bp
plt.scatter(df['nnz_per_row'], df['mflops'], label='CSR', c=df['bp'], cmap='gnuplot2_r', s=12**2, alpha=0.5, marker='^')
plt.clim(min_bp, max_bp)
plt.scatter(df_coo['nnz_per_row'], df_coo['mflops'], label='COO', c=df_coo['bp'], cmap='gnuplot2_r', s=12**2, alpha=0.5, marker='*')
plt.clim(min_bp, max_bp)
cbar = plt.colorbar()
cbar.ax.tick_params(labelsize=15)
cbar.set_label('Branch MisPrediction Percentage Index', size=15)
plt.xlim(0,15)
plt.gca().legend(fancybox=True, framealpha=0.5, scatterpoints=1, fontsize=15, loc='best')
leg = plt.gca().get_legend()
leg.legendHandles[0].set_color('white')
leg.legendHandles[0].set_edgecolor('black')
leg.legendHandles[1].set_color('white')
leg.legendHandles[1].set_edgecolor('black')
plt.savefig(out_dir+'csr_bp_nnz_scatter_plot.png')
'''
plt.xlabel('dia_ratio')
plt.ylabel('Performance (MFLOPS)')
plt.scatter(df_small['ratio_diag'], df_small['mflops'], color='yellowgreen', label='DIA')
if not df_comb_small.empty:
  plt.scatter(df_comb_small['ratio_diag'], df_comb_small['mflops'], label='combination-DIA')
if not df_not_small.empty:
  plt.scatter(df_not_small['ratio_diag'], df_not_small['mflops'], color='deeppink', label='not-DIA')
plt.title('small')
plt.gca().legend(loc=0, scatterpoints = 1)
plt.savefig(out_dir+'small_dia_ratio_scatter_plot.png')

plt.clf()

plt.xlabel('dia_ratio')
plt.ylabel('Performance (MFLOPS)')
plt.scatter(df_xlarge['ratio_diag'], df_xlarge['mflops'], color='yellowgreen')
if not df_comb_xlarge.empty:
  plt.scatter(df_comb_xlarge['ratio_diag'], df_comb_xlarge['mflops'])
if not df_not_xlarge.empty:
  plt.scatter(df_not_xlarge['ratio_diag'], df_not_xlarge['mflops'], color='deeppink')
plt.title('xlarge')
plt.savefig(out_dir+'xlarge_dia_ratio_scatter_plot.png')

plt.clf()

plt.xlabel('dia_ratio')
plt.ylabel('Performance (MFLOPS)')
plt.scatter(df_large['ratio_diag'], df_large['mflops'], color='yellowgreen')
if not df_comb_large.empty:
  plt.scatter(df_comb_large['ratio_diag'], df_comb_large['mflops'])
if not df_not_large.empty:
  plt.scatter(df_not_large['ratio_diag'], df_not_large['mflops'], color='deeppink')
plt.title('large')
plt.savefig(out_dir+'large_dia_ratio_scatter_plot.png')

plt.clf()

plt.xlabel('dia_ratio')
plt.ylabel('Performance (MFLOPS)')
plt.scatter(df_medium['ratio_diag'], df_medium['mflops'], color='yellowgreen')
if not df_comb_medium.empty:
  plt.scatter(df_comb_medium['ratio_diag'], df_comb_medium['mflops'])
if not df_not_medium.empty:
  plt.scatter(df_not_medium['ratio_diag'], df_not_medium['mflops'], color='deeppink')
plt.title('medium')
plt.savefig(out_dir+'medium_dia_ratio_scatter_plot.png')
'''
