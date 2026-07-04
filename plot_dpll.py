import pandas as pd
import plotly.express as px
import plotly.graph_objects as go

df = pd.read_csv('results_dpll.csv')


fig = px.line(df, x = 'pigeons', y = 'time')
fig.update_traces(line_color='red')

fig.show()
