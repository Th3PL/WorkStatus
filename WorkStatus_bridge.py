import serial
import paho.mqtt.client as mqtt
import json
from datetime import datetime
import time

# Configurações
SERIAL_PORT = "/dev/ttyUSB0"  # Porta serial
BAUD_RATE = 115200
MQTT_BROKER = "localhost"
MQTT_TOPIC_STATUS = "office/desk01/status"
MQTT_TOPIC_CMD = "office/desk01/cmd"

# MQTT callbacks
def on_connect(client, userdata, flags, rc, properties=None):
    if rc == 0:
        print("Conectado ao broker MQTT")
        client.subscribe(MQTT_TOPIC_CMD)
        print(f"Inscrito em {MQTT_TOPIC_CMD}")
    else:
        print(f"Falha na conexão MQTT (código {rc})")

def on_message(client, userdata, msg):
    try:
        comando = msg.payload.decode('utf-8')
        print(f"Comando recebido: {comando}")
        # Envia comando para Arduino
        ser.write(f"{comando}\n".encode('utf-8'))
        print(f"Enviado para Arduino: {comando}")
    except Exception as e:
        print(f"Erro ao processar comando: {e}")

# Inicialização MQTT
print("Iniciando cliente MQTT...")
client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2, client_id="WorkStatusBridge")
client.on_connect = on_connect
client.on_message = on_message

print(f"Conectando ao broker MQTT em {MQTT_BROKER}:1883...")
try:
    client.connect(MQTT_BROKER, 1883, 60)
    client.loop_start()
except Exception as e:
    print(f"Erro ao conectar no broker: {e}")
    exit(1)

# Inicialização Serial
print(f"Conectando ao Arduino em {SERIAL_PORT}...")
try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    time.sleep(2)  # Aguarda reset do Arduino
    ser.reset_input_buffer()  # Limpa buffer inicial
    print(f"Arduino conectado em {SERIAL_PORT}")
except Exception as e:
    print(f"Erro ao conectar Arduino: {e}")
    print("Dica: Verifique a porta com 'ls /dev/tty*'")
    exit(1)

# Loop principal
print("Bridge ativo. Aguardando dados do Arduino...")
print("Pressione Ctrl+C para sair")

try:
    while True:
        if ser.in_waiting > 0:
            try:
                linha_raw = ser.readline()
                try:
                    linha = linha_raw.decode('utf-8').strip()
                except UnicodeDecodeError:
                    linha = linha_raw.decode('utf-8', errors='ignore').strip()
                if len(linha) < 5:
                    continue
                print(f"Arduino: {linha}")
                if "=" in linha and ";" in linha:
                    try:
                        dados = {}
                        partes = linha.split(";")
                        for parte in partes:
                            if "=" in parte:
                                chave, valor = parte.split("=", 1)
                                dados[chave.strip()] = valor.strip()
                        if "ID" in dados and "STATE" in dados:
                            dados["timestamp"] = datetime.now().isoformat()
                            payload = json.dumps(dados, ensure_ascii=False)
                            client.publish(MQTT_TOPIC_STATUS, payload)
                            print(f"MQTT: {payload}")
                        else:
                            print("Dados incompletos, ignorando...")
                    except Exception as e:
                        print(f"Erro ao processar dados: {e}")
                else:
                    print("Formato inválido, ignorando...")
            except Exception as e:
                print(f"Erro na leitura serial: {e}")
                ser.reset_input_buffer()  # Limpa buffer
        time.sleep(0.05)  # Delay

except KeyboardInterrupt:
    print("Encerrando bridge...")
    client.loop_stop()
    client.disconnect()
    ser.close()
    print("Desconectado com sucesso!")

except Exception as e:
    print(f"Erro fatal: {e}")
    client.loop_stop()
    client.disconnect()
    ser.close()
