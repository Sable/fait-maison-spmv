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

for index, row in df.iterrows():
  if (int(row['N']) * 8) < MM and (int(row['N']) * 8) >= L3 :
    df_xlarge = df_xlarge.append(row)
  if (int(row['N']) * 8) < L3 and (int(row['N']) * 8) >= L2 :
    df_large = df_large.append(row)
  if (int(row['N']) * 8) < L2 and (int(row['N']) * 8) >= L1 :
    df_medium = df_medium.append(row)
  if (int(row['N']) * 8) < L1 :
    df_small = df_small.append(row)


plt.xlabel('CSR Working Set (bytes)', fontsize=15)
plt.ylabel('Performance (MFLOPS)', fontsize=15)
plt.xticks(fontsize=15)
plt.yticks(fontsize=15)
plt.scatter(df['ws'], df['mflops'], label='_nolegend_', c=df['l'], cmap='gnuplot2_r', s=12**2, alpha=0.5)
cbar = plt.colorbar()
cbar.ax.tick_params(labelsize=15)
cbar.set_label('Cache Misses Percentage Index', size=15)
plt.axvline(x=32*1024, linestyle='--', color='green',label='L1')
plt.axvline(x=256*1024, linestyle='--', color='hotpink', label='L2')
plt.axvline(x=12*1024*1024, linestyle='--',color='orange', label='L3')
plt.xlim(100, 1000000000)
plt.xscale('log')
plt.gca().legend(fancybox=True, framealpha=0.5, scatterpoints=1, fontsize=15, loc='best')
plt.savefig(out_dir+'csr_misses_scatter_plot.png')
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
