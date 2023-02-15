from time import sleep


def inc(regs, reg, amount):
    """Ensure we only have 8 bit values"""
    value = regs[reg] + amount
    regs[reg] = value & 255


def ins_LDA(regs, mem):
    addr = mem[regs["PC"]]
    inc(regs, "PC", 1)
    regs["A"] = mem[addr]


def ins_STA(regs, mem):
    addr = mem[regs["PC"]]
    inc(regs, "PC", 1)
    mem[addr] = regs["A"]


def ins_ADDI(regs, mem):
    value = mem[regs["PC"]]
    inc(regs, "PC", 1)
    inc(regs, "A", value)


def ins_OUTA(regs, mem):
    regs["O"] = regs["A"]


def ins_JMP(regs, mem):
    regs["PC"] = mem[regs["PC"]]


def run_prog(prog):
    mem = prog
    mem.extend([0 for x in range(256 - len(prog))])
    regs = {"PC": 0, "A": 0, "B": 0, "O": 0}
    Globals = globals()
    halt = False

    while not halt:
        ins = mem[regs["PC"]]
        inc(regs, "PC", 1)
        Globals["ins_" + ins](regs, mem)
        print(*[f"{k}: {v}" for k,v in regs.items()], sep="\t")
        sleep(0.1)


prog = [
    "LDA", 9,
    "ADDI", 1,
    "STA", 9,
    "OUTA",
    "JMP", 0,
    0,
]

run_prog(prog)
