import pandas as pd
import plotly.express as px
import plotly.graph_objects as go

df = pd.read_csv('results_dpll.csv')
df2 = pd.read_csv('results_gwsat.csv')



fig1 = px.line(df, x = 'pigeons', y = 'time')
fig1.update_traces(line_color='red')
fig2 = px.line(df2, x='pigeons', y = 'time')
fig2.update_traces(line_color='blue')

fig = go.Figure(data = fig1.data + fig2.data, layout=go.Layout(
        title=go.layout.Title(text="Blau: GWSAT, Rot: DPLL")
    ))

fig.show()
