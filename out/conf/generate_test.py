import json

data = []

for i in range(8004, 9004):
    if i % 2 == 0:
        thmin = 1
        thmax = 50000
    else:
        thmin = "0"
        thmax = "0"

    item = {
        "bus_addr": i,
        "desc": "",
        "rw": "R/W",
        "data_len": 32,
        "data_type": "float",
        "thmin": thmin,
        "thmax": thmax,
        "alarm_level": "0",
        "default_value": 0,
        "modbus_addr": "",
        "device_name": "",
        "decimal": 0,
        "subtype": 2,
        "bit_offset": 0
    }

    data.append(item)

# 写入文件
with open("data.json", "w", encoding="utf-8") as f:
    json.dump(data, f, indent=2, ensure_ascii=False)