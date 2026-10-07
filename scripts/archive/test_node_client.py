#!/usr/bin/env python3
"""
OpenClaw Node Client Test
Connects to OpenClaw gateway as a node.
"""
import asyncio
import websockets
import json
import logging
import sys
from datetime import datetime

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

GATEWAY_URL = "ws://192.168.0.67:18789"
GATEWAY_TOKEN = "852dd5307c7ca17caf4d062c566b71930e636774b3f7aa61"

async def connect_as_node():
    """Connect to OpenClaw gateway as a node"""
    try:
        logger.info(f"Connecting to gateway: {GATEWAY_URL}")
        
        # Connect with auth token
        headers = {
            "Authorization": f"Bearer {GATEWAY_TOKEN}"
        }
        
        async with websockets.connect(
            f"{GATEWAY_URL}/ws",
            extra_headers=headers
        ) as websocket:
            logger.info("Connected to gateway")
            
            # Send initial handshake/identification
            handshake = {
                "type": "node-identify",
                "nodeId": "napcatqq-node",
                "name": "NapCatQQ Node",
                "capabilities": ["message-forward", "notify"],
                "timestamp": datetime.now().isoformat()
            }
            
            await websocket.send(json.dumps(handshake))
            logger.info(f"Sent handshake: {handshake}")
            
            # Wait for response
            response = await websocket.recv()
            logger.info(f"Received: {response}")
            
            # Keep connection alive and listen for messages
            while True:
                try:
                    message = await websocket.recv()
                    data = json.loads(message)
                    logger.info(f"Message from gateway: {data}")
                    
                    # Process message
                    if data.get("type") == "ping":
                        # Respond to ping
                        pong = {
                            "type": "pong",
                            "timestamp": datetime.now().isoformat()
                        }
                        await websocket.send(json.dumps(pong))
                        
                    elif data.get("type") == "command":
                        # Handle command from gateway
                        command = data.get("command")
                        logger.info(f"Received command: {command}")
                        
                        # Respond to command
                        response = {
                            "type": "command-response",
                            "commandId": data.get("commandId"),
                            "result": {"status": "processed", "details": f"Executed {command}"},
                            "timestamp": datetime.now().isoformat()
                        }
                        await websocket.send(json.dumps(response))
                        
                except websockets.exceptions.ConnectionClosed:
                    logger.info("Connection closed")
                    break
                except Exception as e:
                    logger.error(f"Error processing message: {e}")
                    break
                    
    except Exception as e:
        logger.error(f"Connection error: {e}")
        return False
    
    return True

async def send_test_message():
    """Send a test message to the gateway"""
    try:
        async with websockets.connect(
            f"{GATEWAY_URL}/ws",
            extra_headers={"Authorization": f"Bearer {GATEWAY_TOKEN}"}
        ) as websocket:
            # Send a test message
            test_msg = {
                "type": "node-message",
                "nodeId": "napcatqq-node",
                "message": {
                    "type": "qq-message",
                    "content": "Test message from NapCatQQ",
                    "timestamp": datetime.now().isoformat(),
                    "source": "QQ Group 1034620808",
                    "sender": "Teacher 1514988610"
                }
            }
            
            await websocket.send(json.dumps(test_msg))
            logger.info(f"Sent test message: {test_msg}")
            
            # Wait for acknowledgment
            response = await websocket.recv()
            logger.info(f"Response: {response}")
            
    except Exception as e:
        logger.error(f"Error sending test message: {e}")

if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "send":
        asyncio.run(send_test_message())
    else:
        asyncio.run(connect_as_node())