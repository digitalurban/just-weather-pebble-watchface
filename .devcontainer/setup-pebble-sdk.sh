#!/bin/bash

# Pebble SDK Setup Script for Codespaces
echo "🔧 Setting up Pebble SDK for development..."

# Install required packages
sudo apt-get update
sudo apt-get install -y python3-pip python3-dev python3-virtualenv nodejs npm

# Install Pebble SDK
echo "📦 Installing Pebble SDK..."
pip3 install --user pebble-tool

# Install ARM toolchain
echo "🛠️  Installing ARM GCC toolchain..."
sudo apt-get install -y gcc-arm-none-eabi

# Add local bin to PATH for current session
export PATH=$PATH:~/.local/bin

# Add to bashrc for future sessions
echo 'export PATH=$PATH:~/.local/bin' >> ~/.bashrc

# Install Pebble SDK
echo "⬬ Downloading and installing Pebble SDK..."
pebble sdk install latest

# Verify installation
echo "✅ Verifying Pebble SDK installation..."
pebble --version

echo "🎉 Pebble SDK setup complete!"
echo "💡 You can now run 'pebble build' to build your Pebble apps."