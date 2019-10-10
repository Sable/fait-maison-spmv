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
df_xlarge = pd.DataFrame()
df_large = pd.DataFrame()
df_medium = pd.DataFrame()
df_small = pd.DataFrame()

df_comb = pd.read_csv(sys.argv[2], index_col = 'name')
df_comb_xlarge = pd.DataFrame()
df_comb_large = pd.DataFrame()
df_comb_medium = pd.DataFrame()
df_comb_small = pd.DataFrame()

df_not = pd.read_csv(sys.argv[3], index_col = 'name')
df_not = df_not.loc[df_not.ratio_ell > 0]
df_not = df_not.loc[df_not.ratio_ell < 3]
df_not = df_not.loc[df_not.max_nnz_row < 100]
df_not = df_not[~(df_not['format'].str.contains('dia') == True)]
df_not = df_not.loc[df_not['mflops'] > 0]
df_not_xlarge = pd.DataFrame()
df_not_large = pd.DataFrame()
df_not_medium = pd.DataFrame()
df_not_small = pd.DataFrame()

for index, row in df.iterrows():
  if (int(row['N']) * 8) < MM and (int(row['N']) * 8) >= L3 :
    df_xlarge = df_xlarge.append(row)
  if (int(row['N']) * 8) < L3 and (int(row['N']) * 8) >= L2 :
    df_large = df_large.append(row)
  if (int(row['N']) * 8) < L2 and (int(row['N']) * 8) >= L1 :
    df_medium = df_medium.append(row)
  if (int(row['N']) * 8) < L1 :
    df_small = df_small.append(row)


for index, row in df_comb.iterrows():
  if (int(row['N']) * 8) < MM and (int(row['N']) * 8) >= L3 :
    df_comb_xlarge = df_comb_xlarge.append(row)
  if (int(row['N']) * 8) < L3 and (int(row['N']) * 8) >= L2 :
    df_comb_large = df_comb_large.append(row)
  if (int(row['N']) * 8) < L2 and (int(row['N']) * 8) >= L1 :
    df_comb_medium = df_comb_medium.append(row)
  if (int(row['N']) * 8) < L1 :
    df_comb_small = df_comb_small.append(row)

for index, row in df_not.iterrows():
  if (int(row['N']) * 8) < MM and (int(row['N']) * 8) >= L3 :
    df_not_xlarge = df_not_xlarge.append(row)
  if (int(row['N']) * 8) < L3 and (int(row['N']) * 8) >= L2 :
    df_not_large = df_not_large.append(row)
  if (int(row['N']) * 8) < L2 and (int(row['N']) * 8) >= L1 :
    df_not_medium = df_not_medium.append(row)
  if (int(row['N']) * 8) < L1 :
    df_not_small = df_not_small.append(row)

plt.xlabel('ell_ratio', fontsize=15)
plt.ylabel('Performance (MFLOPS)', fontsize=15)
plt.xticks(fontsize=15)
plt.yticks(fontsize=15)
nnz_per_row = np.concatenate([df['max_nnz_row'], df_comb['max_nnz_row'], df_not['max_nnz_row']], axis = 0)
min_nnz_per_row = nnz_per_row.min()
max_nnz_per_row = nnz_per_row.max()
plt.scatter(df['ratio_ell'], df['mflops'], label='ELL', c=df['max_nnz_row'], cmap='gnuplot2_r', s=12**2, alpha=0.5, marker='*')
plt.clim(min_nnz_per_row, max_nnz_per_row)
if not df_comb.empty:
  plt.scatter(df_comb['ratio_ell'], df_comb['mflops'], label='combination-ELL', c=df_comb['max_nnz_row'], cmap='gnuplot2_r', s=12**2, alpha=0.5, marker='o')
  plt.clim(min_nnz_per_row, max_nnz_per_row)
if not df_not.empty:
  plt.scatter(df_not['ratio_ell'], df_not['mflops'], label='not-DIA-not-ELL', c=df_not['max_nnz_row'], cmap='gnuplot2_r', s=12**2, alpha=0.5, marker='^')
  plt.clim(min_nnz_per_row, max_nnz_per_row)
cbar = plt.colorbar()
cbar.ax.tick_params(labelsize=15)
cbar.set_label('max_nnz_per_row', size=15)
plt.gca().legend(fancybox=True, framealpha=0.5, loc=0, fontsize=15, scatterpoints = 1)
leg = plt.gca().get_legend()
leg.legendHandles[0].set_color('white')
leg.legendHandles[0].set_edgecolor('black')
leg.legendHandles[1].set_color('white')
leg.legendHandles[1].set_edgecolor('black')
leg.legendHandles[2].set_color('white')
leg.legendHandles[2].set_edgecolor('black')
plt.ylim([0,3000])
plt.savefig(out_dir+'ell_ratio_scatter_plot.png')

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
