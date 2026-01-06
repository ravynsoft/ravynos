typedef int weirdType;

provider Bar {
        probe count1(weirdType);
};

#pragma D attributes Evolving/Evolving/Common provider Bar args
