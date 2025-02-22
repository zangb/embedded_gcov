import argparse
import pathlib
import re

class gcda_splitter:
    def __init__(self):
        return

    def _parse_filepath(self, content: list):
        """Parses a filepath from a byte list. Parses until the end token .gcda is encountered
        content: list
            List of bytes. The list starts with a '/'
        end_of_file_path = ".gcda"
        """
        end_of_file_path = ".gcda"
        filepath : str = ""
        for i in range(len(content)):
            # terminate early on null-character
            if chr(content[i]) == '\0':
                return filepath
            # check if a byte is a dot and we still have room till the actual file end
            if chr(content[i]) == '.' and (len(content) - i) >= len(end_of_file_path):
                # join the characters and compare if it matches the .gcda token
                token = "".join(chr(c) for c in content[i:i+len(end_of_file_path)])
                if token == end_of_file_path:
                    filepath = content[:i+len(end_of_file_path)]
                    print("Found filepath ", filepath)
                    break
        return filepath

    def split_gcda(self, content: list):
        """Expects a list of bytes from a merged gcda file that is output by the testrun
        content: list
            List of bytes holding all the raw gcda data from a testrun
        Returns a dictionary with key filepath and the value as the actual gcda data
        """
        filepaths = list()
        i = 0
        while i  < len(content):
            # check if we encountered a slash and then check if we have a valid filepath
            if chr(content[i]) == '/':
                file_path = self._parse_filepath(content[i:])
                # if the path is valid increment the loop variable by the filelength
                # to prevent any submatches, like 1. /home/user/file.gcda, 2. /user/file.gcda
                if file_path:
                    filepaths.append({"pos": i, "path": file_path})
                    i += len(file_path)
            i += 1
        if len(filepaths) == 0:
            return

        gcda_files = dict()
        # iterate over the found files
        for i in range(len(filepaths)):
            # if we got the last element extract data from the end of the filepath + terminating null character
            # + 4 Bytes which indicate the length of the gcda data. After this the actual header of the gcda file
            # starts with 12 Byte of magic number, version and crc. Extract until 9 Bytes before the end because
            # the gcda file ends with "Gcov End\0"
            data = list()
            if i == (len(filepaths) - 1):
                data = content[filepaths[i]["pos"] + len(filepaths[i]["path"]) + 5: len(content)-9]
            else:
                # extract data that starting from 5 Bytes after the filepath end until the start of the next filepath
                data = \
                    content[filepaths[i]["pos"] + len(filepaths[i]["path"]) + 5 : filepaths[i+1]["pos"]]

            #data = b''.join([bytes([b]) for b in data if (b == 9 or b == 10 or b == 13 or (b>31 and b <127))])

            gcda_files[filepaths[i]["path"]] = data
        return gcda_files

if __name__ == "__main__":
    parser  = argparse.ArgumentParser()
    parser.add_argument("-p", "--path", help="The path to the raw gcov output from the target application")
    args = parser.parse_args()
    if not args.path:
        raise Exception("Missing filepath for gcov input file")


    splitter = gcda_splitter()
    content = ""

    with open(args.path, "rb") as f:
        content = f.read()

    gcdas = splitter.split_gcda(content)

    for path, data in gcdas.items():
        with open(path, "wb") as f:
            f.write(data)
    print("Done splitting GCDA files\n\n")