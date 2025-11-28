 ___________      ________      ____________
|           |    |        |    |            |
| Websocket | -> | Parser | -> | Aggregator |
|___________|    |________|    |____________|

Websocket:
- Main() will run a single websocket thread of a combined binance stream
- Recieve data in candles, of multiple times frames, of multiple pairs
- Push raw data in thread safe queue

Parser:
- Pop raw data and parse JSON
- Gather needed data (ohlc, open/closed, volume)
- Convert types and push to thread safe queue

Aggregator:
- Pop organized data and begin analysis
- Call database write function to store closed candles
- Keep track of live candle data for real time charting updates