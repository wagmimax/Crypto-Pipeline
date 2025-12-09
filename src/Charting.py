import pandas as pd
import numpy as np
import mplfinance as mpf
import matplotlib.animation as animation
import matplotlib.pyplot as plt

# Initial data: 30 candles
timestamps = pd.date_range('2025-12-08 20:00', periods=30, freq='S')
data = pd.DataFrame({
    'Open': np.random.rand(30)*100 + 1000,
    'High': np.random.rand(30)*100 + 1050,
    'Low': np.random.rand(30)*100 + 950,
    'Close': np.random.rand(30)*100 + 1000
}, index=timestamps)

# Matplotlib figure and axis
fig, ax = plt.subplots(figsize=(12,6))

# mplfinance style
mc = mpf.make_marketcolors(up='green', down='red', inherit=True)
s = mpf.make_mpf_style(marketcolors=mc)

# Keep track of intra-candle updates
intra_updates = 0
max_intra_updates = 5  # update each candle 5 times before adding a new one

def update(frame):
    global data, intra_updates
    last_candle = data.iloc[-1].copy()
    
    # Modify last candle slightly to simulate price changes
    last_candle['High'] = max(last_candle['High'], last_candle['Close'] + np.random.rand()*5)
    last_candle['Low'] = min(last_candle['Low'], last_candle['Close'] - np.random.rand()*5)
    last_candle['Close'] = last_candle['Close'] + np.random.randn()  # random walk
    
    # Replace the last candle with updated values
    data.iloc[-1] = last_candle

    intra_updates += 1
    if intra_updates >= max_intra_updates:
        # Add a new candle
        new_close = data['Close'].iloc[-1] + np.random.randn()*2
        new_row = pd.DataFrame({
            'Open': [data['Close'].iloc[-1]],
            'High': [data['Close'].iloc[-1] + np.random.rand()*5],
            'Low': [data['Close'].iloc[-1] - np.random.rand()*5],
            'Close': [new_close]
        }, index=[pd.Timestamp.now()])
        data = pd.concat([data, new_row]).tail(30)  # keep last 30 candles
        intra_updates = 0  # reset counter

    # Redraw candlestick chart
    ax.clear()
    mpf.plot(data, type='candle', ax=ax, style=s, show_nontrading=True)
    
    # Add horizontal line at current close price
    current_close = data['Close'].iloc[-1]
    ax.axhline(y=current_close, color='blue', linestyle='--', alpha=0.5)

    # Add text label showing current close price
    # Positioned at the right edge of the chart
    ax.text(
        x=data.index[-1], 
        y=current_close, 
        s=f"{current_close:.2f}", 
        color='blue', 
        fontsize=12, 
        fontweight='bold',
        va='bottom',  # vertical alignment
        ha='left'     # horizontal alignment
    )

    ax.set_title("Live Candlesticks with Current Close Price")

ani = animation.FuncAnimation(fig, update, interval=20)  # update every 200ms
plt.show()
