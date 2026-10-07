#!/usr/bin/env node

const WebSocket = require('ws');

const GATEWAY_URL = 'ws://192.168.0.67:18789';
const GATEWAY_TOKEN = '852dd5307c7ca17caf4d062c566b71930e636774b3f7aa61';

async function connectAsNode() {
  console.log('Connecting to gateway as node...');
  
  const ws = new WebSocket(GATEWAY_URL, {
    headers: {
      'Authorization': `Bearer ${GATEWAY_TOKEN}`
    }
  });

  ws.on('open', function open() {
    console.log('WebSocket connection opened');
    
    // Send connect request
    const connectRequest = {
      id: 'connect-' + Date.now(),
      type: 'req',
      method: 'connect',
      params: {
        client: {
          id: 'node-host',
          mode: 'node',
          version: '1.0.0',
          platform: 'linux',
          displayName: 'NapCatQQ Node'
        },
        minProtocol: 3,
        maxProtocol: 3,
        auth: {
          token: GATEWAY_TOKEN
        },
        caps: ['message-forward', 'notify'],
        scopes: ['node']
      }
    };
    
    console.log('Sending connect request:', JSON.stringify(connectRequest, null, 2));
    ws.send(JSON.stringify(connectRequest));
  });

  ws.on('message', function incoming(data) {
    try {
      const message = JSON.parse(data);
      console.log('Received:', JSON.stringify(message, null, 2));
      
      // Handle different message types
      if (message.type === 'event') {
        console.log(`Event: ${message.event}`);
        
        if (message.event === 'connect.challenge') {
          // Respond to challenge
          console.log('Responding to challenge');
          const challengeResponse = {
            id: 'challenge-' + Date.now(),
            type: 'req',
            method: 'connect.respond',
            params: {
              nonce: message.payload.nonce,
              ts: message.payload.ts,
              // For token auth, maybe just echo back
              signed: message.payload.nonce // This might need actual signing
            }
          };
          ws.send(JSON.stringify(challengeResponse));
        } else if (message.event === 'node.pair.requested') {
          // Handle pairing request
          console.log('Pairing request received');
          
          // In real scenario, you'd need to approve this via CLI
          // For now just acknowledge
          const ack = {
            id: 'ack-' + Date.now(),
            type: 'req',
            method: 'node.pair.ack',
            params: {
              requestId: message.payload?.requestId
            }
          };
          ws.send(JSON.stringify(ack));
        }
      } else if (message.type === 'res' && message.id) {
        console.log(`Response for ${message.id}: ${message.ok ? 'OK' : 'Error'}`);
      }
      
    } catch (e) {
      console.log('Raw message:', data.toString());
    }
  });

  ws.on('error', function error(err) {
    console.error('WebSocket error:', err);
  });

  ws.on('close', function close() {
    console.log('WebSocket connection closed');
  });
}

// Check if WebSocket library is available
try {
  require('ws');
  connectAsNode();
} catch (e) {
  console.error('WebSocket library not installed. Installing...');
  const { execSync } = require('child_process');
  try {
    execSync('npm install ws', { stdio: 'inherit' });
    console.log('WebSocket library installed. Please run again.');
  } catch (installError) {
    console.error('Failed to install WebSocket library:', installError);
  }
}