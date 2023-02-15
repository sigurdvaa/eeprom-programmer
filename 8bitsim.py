from time import sleep


def inc(regs, reg, amount):
    """Ensure we only have 8 bit values"""
    value = regs[reg] + amount
    regs[reg] = value & 255


def ins_LDA(regs, mem, flags):
    addr = mem[regs["PC"]]
    inc(regs, "PC", 1)
    regs["A"] = mem[addr]


def ins_LDB(regs, mem, flags):
    addr = mem[regs["PC"]]
    inc(regs, "PC", 1)
    regs["B"] = mem[addr]


def ins_LDIA(regs, mem, flags):
    value = mem[regs["PC"]]
    inc(regs, "PC", 1)
    regs["A"] = value


def ins_STA(regs, mem, flags):
    addr = mem[regs["PC"]]
    inc(regs, "PC", 1)
    mem[addr] = regs["A"]


def ins_STB(regs, mem, flags):
    addr = mem[regs["PC"]]
    inc(regs, "PC", 1)
    mem[addr] = regs["B"]


def ins_ADD(regs, mem, flags):
    addr = mem[regs["PC"]]
    inc(regs, "PC", 1)
    value = mem[addr]
    flags["C"] = regs["A"] + value > 255
    flags["Z"] = regs["A"] + value == 0
    inc(regs, "A", value)


def ins_ADDI(regs, mem, flags):
    value = mem[regs["PC"]]
    inc(regs, "PC", 1)
    flags["C"] = regs["A"] + value > 255
    flags["Z"] = regs["A"] + value == 0
    inc(regs, "A", value)


def ins_OUTA(regs, mem, flags):
    regs["O"] = regs["A"]


def ins_OUTI(regs, mem, flags):
    value = mem[regs["PC"]]
    inc(regs, "PC", 1)
    regs["O"] = value


def ins_JMP(regs, mem, flags):
    regs["PC"] = mem[regs["PC"]]


def ins_JMPC(regs, mem, flags):
    value = mem[regs["PC"]]
    inc(regs, "PC", 1)
    if flags["C"]:
        regs["PC"] = value


def run_prog(prog):
    mem = prog
    mem.extend([0 for x in range(256 - len(prog))])
    regs = {"PC": 0, "A": 0, "B": 0, "O": 0}
    flags = {"C": False, "Z": False}
    Globals = globals()
    halt = False

    while not halt:
        ins = mem[regs["PC"]]
        inc(regs, "PC", 1)
        Globals["ins_" + ins](regs, mem, flags)
        print(*[f"{k}: {v}" for k,v in regs.items()], sep="\t")
        sleep(0.1)


add_NUM = 9
add = [
    "OUTA",
    "ADDI", 1,
    "JMP", 0,
]

fib_TMP = 21
fib_OLD = 22
fib = [
    "OUTA",
    "STA", fib_TMP,
    "ADD", fib_OLD,
    "LDB", fib_TMP,
    "STB", fib_OLD,
    "JMPC", 13,
        "JMP", 0,
    "OUTI", 0,
    "LDIA", 1,
    "STA", fib_OLD,
    "JMP", 0,
    0, # fib_TMP
    1, # fib_OLD
]

run_prog(fib)
