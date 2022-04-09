import random

size_names = ['small', 'medium', 'big']
sizes = [100, 5000, 100000]
filenames = []

files_per_size = 1


def create_string(lines):
    text = ''
    for line in range(lines):
        word_count = random.randint(5, 30)
        for wc in range(word_count):
            letter_count = random.randint(1, 15)
            for lc in range(letter_count):
                text += random.choice("abcdefghijklmnopqrstuwvxyz")
            text += " "
        text += '\n'
    return text


for i in range(3):
    for j in range(files_per_size):
        filename = "./files/"+size_names[i]+str(j)+".txt"
        filenames.append(filename)
        with open(filename, 'w') as file:
            text = create_string(sizes[i])
            file.write(text)

# zliczanie, rozne rozmiary, rozna ilosc
text = ''
for sizename in size_names:
    for count in range(1, 11):
        # tworzenie tabeli
        text += "--create_table\n10\n"
        text += "--print\n"
        text += "size="+str(sizename)+"_count="+str(count)+"\n"
        # czas wc_files
        text += "--print\ncounting:\n"
        text += "--start_clock\n"
        text += "--wc_files\n"
        for c in range(count):
            text += "../files/"+sizename+"0.txt\n"
        text += "--stop_clock\n"
        # # zliczenie czasu ladowania
        text += "--print\nloading:\n"
        text += "--start_clock\n"
        text += "--load_file\n"
        text += "--stop_clock\n"
        # usuwanie tabeli
        text += "--print\nremoving_blocks\n"
        text += "--start_clock\n"
        text += "--remove_table\n"
        text += "--stop_clock\n"
        text += "--print\n..........\n"

with open("./files/arg1.txt", 'w') as file:
    file.write(text)

text = ''
for sizename in size_names:
    for count in range(1, 11):
        text += "--print\nsize="+sizename+"_count="+str(count)+"\n"
        text += "--start_clock\n"
        text += "--create_table\n10\n--remove_table\n"
        text += "--stop_clock\n"
        text += "--print\n..........\n"

with open("./files/arg2.txt", "w") as file:
    file.write(text)




