port="$(python -c 'import random; print(random.randint(10000,40000))')"
ip="$(hostname )" 
# hostname -i for ip
echo "my ip: $ip"
./fbsd "$ip" "$port"
