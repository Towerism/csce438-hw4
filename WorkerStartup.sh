git pull
make -j 15
port="$(python -c 'import random; print(random.randint(10000,40000))')"
./fbsd "$port"
