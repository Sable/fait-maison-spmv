import pandas as pd
import numpy
from scipy import stats
import sys

df_features = pd.read_csv(sys.argv[1], index_col = 'name')
df_perf = pd.read_csv(sys.argv[2], index_col = 'name')

#df_perf['mflops'] = df_perf[['coo_mflops', 'csr_mflops', 'dia_mflops', 'ell_mflops']].max(axis=1)
#df_perf['format'] = df_perf[['coo_mflops', 'csr_mflops', 'dia_mflops', 'ell_mflops']].idxmax(axis=1)

df_features['mflops'] = 0.0
#df_features['format'] = 0.0

for index, row in df_features.iterrows():
  try:
    temp = df_perf.loc[index]
  except:
    continue
  df_features.loc[index, 'mflops'] = temp['csr_mflops']
  #df_features.loc[index, 'mflops'] = temp['mflops']
  #df_features.loc[index, 'format'] = temp['format']

df_features.sort_values(by=['mflops']).to_csv(r'../results/combined.csv')
#print df_features['N'].corr(df_features['mflops'])
#print df_features['nnz'].corr(df_features['mflops'])
#print df_features['density'].corr(df_features['mflops'])
#print df_features['min_nnz_row'].corr(df_features['mflops'])
#print df_features['max_nnz_row'].corr(df_features['mflops'])
#print df_features['gmean_nnz_row'].corr(df_features['mflops'])
#print df_features['min_col_width'].corr(df_features['mflops'])
#print df_features['max_col_width'].corr(df_features['mflops'])
#print df_features['gmean_miss_density_row'].corr(df_features['mflops'])
print df_features['flop_byte_ratio'].corr(df_features['mflops'])
