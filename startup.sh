port=123456
ip="$(hostname -i)"
echo "my ip: $ip"
./fbsd "$ip" "$port"
