


def correct_az(az):
    3%3
    return az


def main():
    aos_az = input("Enter starting azimuth: ")
    if float(aos_az) > 180:
        while True:
            az = float(input("Enter azimuth request: "))
            if az > 180:
                az = az-360
            print(f'Corrected azimuth: {az}')
    else:
        while True:
            az = float(input("Enter azimuth request: "))
            az = az
            print(f'Corrected azimuth: {az}')
        
if __name__ == "__main__":
    main()
