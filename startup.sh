port="$(python -c 'import random; print(random.randint(10000,40000))')"
ip="$(hostname -i)"
echo "my ip: $ip"
./fbsd "$ip" "$port"
