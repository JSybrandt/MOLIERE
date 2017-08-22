#!/software/python/3.4/bin/python

import argparse


class Profile:
    def __init__(self, name):
        self.name = name
        self.attr = {}

    def get(self, attrName):
        if attrName in self.attr:
            return self.attr[attrName]
        else:
            return None

    def set(self, attrName, value):
        self.attr[attrName] = value

    def __str__(self):
        return self.name


def GetProfiles(profilePath):
    with open(profilePath, "r") as pFile:
        profile = None
        for line in pFile:
            tokens = line.split()
            if len(tokens) == 1:
                if profile is not None:
                    yield profile
                profile = Profile(tokens[0])
            else:
                try:
                    profile.set(tokens[0], float(tokens[1]))
                except:
                    pass


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-p", "--profile",
                        action="store",
                        dest="profilePath",
                        help="File path of profile data.")
    parser.add_argument("-a", "--attribute",
                        action="store",
                        dest="attrName",
                        help="Attribute name on which to sort.")
    parser.add_argument("-v", "--verbose",
                        action="store_true",
                        dest="verbose",
                        help="print debug info")

    args = parser.parse_args()
    verbose = args.verbose
    profilePath = args.profilePath
    attrName = args.attrName

    profiles = [p for p in GetProfiles(profilePath)]

    profiles.sort(key=lambda x:  x.get(attrName), reverse=True)

    for profile in profiles:
        print(profile, " ", profile.get(attrName))


if __name__ == "__main__":
    main()
