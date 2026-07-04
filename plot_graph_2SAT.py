import pandas as pd
import plotly.graph_objects as go

df_dpll = pd.read_csv("results_dpll.csv")
df_gwsat = pd.read_csv("results_gwsat.csv")
df_2sat = pd.read_csv("results_2SAT.csv")

fig = go.Figure()

fig.add_trace(
    go.Scatter(
        x=df_dpll["pigeons"],
        y=df_dpll["time"],
        mode="lines+markers",
        name="DPLL",
        line=dict(color="red")
    )
)

fig.add_trace(
    go.Scatter(
        x=df_gwsat["pigeons"],
        y=df_gwsat["time"],
        mode="lines+markers",
        name="GWSAT",
        line=dict(color="blue")
    )
)

fig.add_trace(
    go.Scatter(
        x=df_2sat["pigeons"],
        y=df_2sat["time"],
        mode="lines+markers",
        name="2SAT",
        line=dict(color="green")
    )
)

fig.update_layout(
    title="Runtime Comparison",
    xaxis_title="Size CNF",
    yaxis_title="Time (s)",
    legend_title="Algorithm"
)

fig.show()