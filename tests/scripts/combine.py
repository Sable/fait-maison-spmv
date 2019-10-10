import pandas as pd
import numpy
from scipy import stats
import sys

def siblings(df):
  count = 0
  format_list = ['coo_mflops', 'csr_mflops', 'dia_mflops', 'ell_mflops']
  df['mflops'] = df[format_list].max(axis=1)
  df['format'] = df[format_list].idxmax(axis=1)
  my_df = df.copy(deep=True)
  for index, row in df.iterrows():
    my_df.loc[index, 'format'] = row['format'][0:3] 
    sd1 = row['format'][0:4] + 'sd'
    if row[sd1] == 'None':
      optimal_min = float(row['mflops'])
    else:
      optimal_min = float(row['mflops']) + float(row[sd1])
    optimal_perfdiff = (1 - 10/100.0) * float(row['mflops'])    
    temp_list = list(format_list)
    temp_list.remove(row['format'])
    while temp_list :
      max2 = row[temp_list].max()
      sd2 = row[temp_list].idxmax()[0:4] + 'sd'
      if row[sd2] == 'None':
        optimal2_max = max2
      else:
        optimal2_max = max2 + float(row[sd2])
      if not((optimal_min > optimal2_max) and (optimal2_max <= optimal_perfdiff)) :
        my_df.loc[index, 'format'] += '_' + row[temp_list].idxmax()[0:3] 
        temp_list.remove(row[temp_list].idxmax())
      else :
        break
  return my_df
    
df_features = pd.read_csv(sys.argv[1], index_col = 'name')
df_features = df_features[~df_features.index.duplicated()]
df_perf = pd.read_csv(sys.argv[2], index_col = 'name')
df_perf = df_perf[~df_perf.index.duplicated()]
df_counters = pd.read_csv(sys.argv[3], index_col = 'name')
df_counters = df_counters[~df_counters.index.duplicated()]
L1 = 32 * 1024
L2 = 256 * 1024
L3 = 12*1024*1024
MM = 16*1024*1024*1024

df_single = siblings(df_perf)

df_features['mflops'] = 0.0
df_features['format'] = ''
df_features['bp'] = 0.0
df_features['l'] = 0.0

for index, row in df_features.iterrows():
  try:
    temp = df_single.loc[index]
    temp2 = df_counters.loc[index]
  except:
    continue
  df_features.loc[index, 'mflops'] = temp['mflops']
  df_features.loc[index, 'format'] = temp['format']
  df_features.loc[index, 'ratio_ell'] = row['ratio_ell']/2
  if temp['format'] == '':
    print(index)
  #if temp['format'] == 'coo':
    #df_features.loc[index, 'bp'] = temp2['csr_bp']
  #  if (int(temp['N']) * 8) < MM and (int(temp['N']) * 8) >= L3 :
   #   df_features.loc[index, 'l'] = (temp2['coo_l3'] * 100)/(temp['outer']*temp['inner']*temp['nnz'])
    #if (int(temp['N']) * 8) < L3 and (int(temp['N']) * 8) >= L2 :
     # df_features.loc[index, 'l'] = (temp2['coo_l2'] * 100)/(temp['outer']*temp['inner']*temp['nnz'])
    #if (int(temp['N']) * 8) < L2 and (int(temp['N']) * 8) >= L1 :
     # df_features.loc[index, 'l'] = (temp2['coo_l1'] * 100)/(temp['outer']*temp['inner']*temp['nnz'])
  if temp['format'] == 'csr':
    df_features.loc[index, 'bp'] = temp2['csr_bp']
    if (int(temp['N']) * 8) < MM and (int(temp['N']) * 8) >= L3 :
      df_features.loc[index, 'l'] = (temp2['csr_l3'] * 100)/(temp['outer']*temp['inner']*temp['nnz'])
    if (int(temp['N']) * 8) < L3 and (int(temp['N']) * 8) >= L2 :
      df_features.loc[index, 'l'] = (temp2['csr_l2'] * 100)/(temp['outer']*temp['inner']*temp['nnz'])
    if (int(temp['N']) * 8) < L2 and (int(temp['N']) * 8) >= L1 :
      df_features.loc[index, 'l'] = (temp2['csr_l1'] * 100)/(temp['outer']*temp['inner']*temp['nnz'])
  #if temp['format'] == 'dia':
   #df_features.loc[index, 'bp'] = temp2['dia_bp']
    #if (int(temp['N']) * 8) < MM and (int(temp['N']) * 8) >= L3 :
     # df_features.loc[index, 'l'] = (temp2['dia_l3'] * 100)/(temp['outer']*temp['inner']*temp['nnz'])
   # if (int(temp['N']) * 8) < L3 and (int(temp['N']) * 8) >= L2 :
    #  df_features.loc[index, 'l'] = (temp2['dia_l2'] * 100)/(temp['outer']*temp['inner']*temp['nnz'])
   # if (int(temp['N']) * 8) < L2 and (int(temp['N']) * 8) >= L1 :
    #  df_features.loc[index, 'l'] = (temp2['dia_l1'] * 100)/(temp['outer']*temp['inner']*temp['nnz'])
  #if temp['format'] == 'ell':
    #df_features.loc[index, 'bp'] = temp2['ell_bp']

dia = df_features.loc[df_features.format == 'dia']
dia['ws'] = (dia.num_diags + 2 * dia.N + dia.num_diag_elems) * 4
dia['perf'] = dia.ratio_diag * dia.mflops
small_dia = dia.loc[dia.ws < 32768] 
medium_dia = dia.loc[(dia.ws > 32768) & (dia.ws < 262144)] 
large_dia = dia.loc[(dia.ws > 262144) & (dia.ws < 12582912 )] 
xlarge_dia = dia.loc[dia.ws > 12582912] 
dia.sort_values(by=['ratio_diag'], ascending=False).to_csv(r'../results/combined_dia.csv')
small_dia.sort_values(by=['ws']).to_csv(r'../results/combined_small_dia.csv')
medium_dia.sort_values(by=['ws']).to_csv(r'../results/combined_medium_dia.csv')
large_dia.sort_values(by=['ws']).to_csv(r'../results/combined_large_dia.csv')
xlarge_dia.sort_values(by=['ws']).to_csv(r'../results/combined_xlarge_dia.csv')
not_dia = df_features[~df_features['format'].str.contains('dia')]
not_dia.sort_values(by=['ratio_diag'], ascending=False).to_csv(r'../results/combined_not_dia.csv')
comb_dia = df_features[df_features['format'].str.contains('dia_|_dia')]
comb_dia.sort_values(by=['ratio_diag'], ascending=False).to_csv(r'../results/combined_comb_dia.csv')

ell = df_features.loc[df_features.format == 'ell']
ell['ws'] = (2 * ell.N + ell.num_ell_elems) * 4
small_ell = ell.loc[ell.ws < 32768] 
medium_ell = ell.loc[(ell.ws > 32768) & (ell.ws < 262144)] 
large_ell = ell.loc[(ell.ws > 262144) & (ell.ws < 12582912)] 
xlarge_ell = ell.loc[ell.ws > 12582912] 
ell.sort_values(by=['max_nnz_row'],ascending=False).to_csv(r'../results/combined_ell.csv')
small_ell.sort_values(by=['ws']).to_csv(r'../results/combined_small_ell.csv')
medium_ell.sort_values(by=['ws']).to_csv(r'../results/combined_medium_ell.csv')
large_ell.sort_values(by=['ws']).to_csv(r'../results/combined_large_ell.csv')
xlarge_ell.sort_values(by=['ws']).to_csv(r'../results/combined_xlarge_ell.csv')
not_ell = df_features[df_features['ratio_ell'] > 0]
not_ell = not_ell[~not_ell['format'].str.contains('ell')]
not_ell.sort_values(by=['max_nnz_row'], ascending=False).to_csv(r'../results/combined_not_ell.csv')
comb_ell = df_features[df_features['format'].str.contains('ell_|_ell')]
comb_ell.sort_values(by=['max_nnz_row'], ascending=False).to_csv(r'../results/combined_comb_ell.csv')

coo = df_features.loc[df_features.format == 'coo']
coo['ws'] = (2 * coo.N + 3 * coo.nnz) * 4
small_coo = coo.loc[coo.ws < 32768] 
medium_coo = coo.loc[(coo.ws > 32768) & (coo.ws < 262144)] 
large_coo = coo.loc[(coo.ws > 262144) & (coo.ws < 12582912)] 
xlarge_coo = coo.loc[coo.ws > 12582912] 
small_coo.sort_values(by=['ws']).to_csv(r'../results/combined_small_coo.csv')
medium_coo.sort_values(by=['ws']).to_csv(r'../results/combined_medium_coo.csv')
large_coo.sort_values(by=['ws']).to_csv(r'../results/combined_large_coo.csv')
xlarge_coo.sort_values(by=['ws']).to_csv(r'../results/combined_xlarge_coo.csv')
coo.sort_values(by=['ws'], ascending=False).to_csv(r'../results/combined_coo.csv')
not_coo = df_features[~df_features['format'].str.contains('coo')]
not_coo.sort_values(by=['nnz_per_row']).to_csv(r'../results/combined_not_coo.csv')
comb_coo = df_features[df_features['format'].str.contains('coo_|_coo')]
comb_coo.sort_values(by=['nnz_per_row']).to_csv(r'../results/combined_comb_coo.csv')

csr = df_features.loc[df_features.format == 'csr']
csr = csr.loc[csr.csr_reuse_distance <= 100]
csr['ws'] = (3 * csr.N + 2 * csr.nnz) * 4
small_csr = csr.loc[csr.ws < 32768] 
medium_csr = csr.loc[(csr.ws > 32768) & (csr.ws < 262144)] 
large_csr = csr.loc[(csr.ws > 262144) & (csr.ws < 12582912)] 
xlarge_csr = csr.loc[csr.ws > 12582912] 
small_csr.sort_values(by=['csr_reuse_distance']).to_csv(r'../results/combined_small_csr.csv')
medium_csr.sort_values(by=['csr_reuse_distance']).to_csv(r'../results/combined_medium_csr.csv')
large_csr.sort_values(by=['csr_reuse_distance']).to_csv(r'../results/combined_large_csr.csv')
xlarge_csr.sort_values(by=['csr_reuse_distance']).to_csv(r'../results/combined_xlarge_csr.csv')
csr.sort_values(by=['csr_reuse_distance'], ascending=False).to_csv(r'../results/combined_csr.csv')
#print df_features['N'].corr(df_features['mflops'])
#print df_features['nnz'].corr(df_features['mflops'])
#print df_features['density'].corr(df_features['mflops'])
#print df_features['min_nnz_row'].corr(df_features['mflops'])
#print df_features['max_nnz_row'].corr(df_features['mflops'])
#print df_features['gmean_nnz_row'].corr(df_features['mflops'])
#print df_features['min_col_width'].corr(df_features['mflops'])
#print df_features['max_col_width'].corr(df_features['mflops'])
#print df_features['gmean_miss_density_row'].corr(df_features['mflops'])
#print df_features['gmean_scatter_ratio'].corr(df_features['mflops'])
#print df_features['flop_byte_ratio'].corr(df_features['mflops'])

df_features.sort_values(by=['ratio_ell'], ascending=False).to_csv(r'../results/combined.csv')
