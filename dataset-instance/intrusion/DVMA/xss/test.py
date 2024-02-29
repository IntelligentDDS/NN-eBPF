with open('images.txt', 'r') as f:
    for line in f.readlines():
        print(f'http://33.33.33.220/image/{line[1:-1]}')