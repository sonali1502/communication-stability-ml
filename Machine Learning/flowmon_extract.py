import xml.etree.ElementTree as ET
import pandas as pd

def ns_to_ms(ns_string):
    try:
        return float(ns_string.replace("+", "").replace("ns", "")) / 1e6
    except:
        return 0

tree = ET.parse("urban_flowmon.xml")
root = tree.getroot()

data = []

for flow in root.iter('Flow'):

    txPackets = int(flow.get('txPackets', 0))
    rxPackets = int(flow.get('rxPackets', 0))
    lostPackets = int(flow.get('lostPackets', 0))

    if txPackets == 0:
        continue

    delaySum = ns_to_ms(flow.get('delaySum', '+0.0ns'))
    jitterSum = ns_to_ms(flow.get('jitterSum', '+0.0ns'))

    pdr = (rxPackets / txPackets) * 100

    avgDelay = 0
    avgJitter = 0

    if rxPackets > 0:
        avgDelay = delaySum / rxPackets
        avgJitter = jitterSum / rxPackets

    data.append([
        flow.get('flowId'),
        txPackets,
        rxPackets,
        lostPackets,
        pdr,
        avgDelay,
        avgJitter
    ])

df = pd.DataFrame(data, columns=[
    "FlowID",
    "TxPackets",
    "RxPackets",
    "LostPackets",
    "PDR",
    "Delay_ms",
    "Jitter_ms"
])

df.to_csv("urban_metrics.csv", index=False)

print("Total Flows =", len(df))
print("Successful Flows =", len(df[df["RxPackets"] > 0]))
print("Average PDR =", round(df["PDR"].mean(),2))
print("CSV Generated Successfully")
