from time import sleep
from typing import Union, Optional
from enum import IntEnum, auto


class Ins(IntEnum):
    HLT = auto()
    LDA = auto()
    LDB = auto()
    LDIA = auto()
    LDIB = auto()
    STA = auto()
    STB = auto()
    ADD = auto()
    ADDI = auto()
    SUB = auto()
    SUBI = auto()
    OUTA = auto()
    OUTI = auto()
    JMP = auto()
    JMPC = auto()
    JMPNC = auto()
    JMPZ = auto()


def inc(regs: dict[str, int], reg: str, amount: int):
    """Ensure we only have 8 bit values"""
    value = regs[reg] + amount
    regs[reg] = value & 255


def ins_HLT(regs: dict[str, int], mem: list[int], flags: dict[str, bool]):
    return True


def ins_LDA(regs: dict[str, int], mem: list[int], flags: dict[str, bool]):
    addr = mem[regs["PC"]]
    assert isinstance(addr, int)
    value = mem[addr]
    assert isinstance(value, int)
    inc(regs, "PC", 1)
    regs["A"] = value


def ins_LDB(regs: dict[str, int], mem: list[int], flags: dict[str, bool]):
    addr = mem[regs["PC"]]
    assert isinstance(addr, int)
    value = mem[addr]
    assert isinstance(value, int)
    inc(regs, "PC", 1)
    regs["B"] = value


def ins_LDIA(regs: dict[str, int], mem: list[int], flags: dict[str, bool]):
    value = mem[regs["PC"]]
    inc(regs, "PC", 1)
    assert isinstance(value, int)
    regs["A"] = value


def ins_LDIB(regs: dict[str, int], mem: list[int], flags: dict[str, bool]):
    value = mem[regs["PC"]]
    inc(regs, "PC", 1)
    assert isinstance(value, int)
    regs["B"] = value


def ins_STA(regs: dict[str, int], mem: list[int], flags: dict[str, bool]):
    addr = mem[regs["PC"]]
    assert isinstance(addr, int)
    inc(regs, "PC", 1)
    mem[addr] = regs["A"]


def ins_STB(regs: dict[str, int], mem: list[int], flags: dict[str, bool]):
    addr = mem[regs["PC"]]
    assert isinstance(addr, int)
    inc(regs, "PC", 1)
    mem[addr] = regs["B"]


def ins_ADD(regs: dict[str, int], mem: list[int], flags: dict[str, bool]):
    addr = mem[regs["PC"]]
    inc(regs, "PC", 1)
    assert isinstance(addr, int)
    value = mem[addr]
    assert isinstance(value, int)
    regs["B"] = value
    flags["C"] = regs["A"] + value > 255
    inc(regs, "A", value)
    flags["Z"] = regs["A"] == 0


def ins_ADDI(regs: dict[str, int], mem: list[int], flags: dict[str, bool]):
    value = mem[regs["PC"]]
    assert isinstance(value, int)
    regs["B"] = value
    inc(regs, "PC", 1)
    flags["C"] = regs["A"] + value > 255
    inc(regs, "A", value)
    flags["Z"] = regs["A"] == 0


def ins_SUB(regs: dict[str, int], mem: list[int], flags: dict[str, bool]):
    addr = mem[regs["PC"]]
    inc(regs, "PC", 1)
    assert isinstance(addr, int)
    value = mem[addr]
    assert isinstance(value, int)
    regs["B"] = value
    flags["C"] = regs["A"] - value > -1
    inc(regs, "A", -value)
    flags["Z"] = regs["A"] == 0


def ins_OUTA(regs: dict[str, int], mem: list[int], flags: dict[str, bool]):
    regs["O"] = regs["A"]


def ins_OUTI(regs: dict[str, int], mem: list[int], flags: dict[str, bool]):
    value = mem[regs["PC"]]
    inc(regs, "PC", 1)
    assert isinstance(value, int)
    regs["O"] = value


def ins_JMP(regs: dict[str, int], mem: list[int], flags: dict[str, bool]):
    value = mem[regs["PC"]]
    assert isinstance(value, int)
    regs["PC"] = value


def ins_JMPC(regs: dict[str, int], mem: list[int], flags: dict[str, bool]):
    value = mem[regs["PC"]]
    inc(regs, "PC", 1)
    if flags["C"]:
        assert isinstance(value, int)
        regs["PC"] = value


def ins_JMPNC(regs: dict[str, int], mem: list[int], flags: dict[str, bool]):
    value = mem[regs["PC"]]
    inc(regs, "PC", 1)
    old = regs["PC"]
    assert isinstance(value, int)
    regs["PC"] = value
    if flags["C"]:
        regs["PC"] = old


def ins_JMPZ(regs: dict[str, int], mem: list[int], flags: dict[str, bool]):
    value = mem[regs["PC"]]
    inc(regs, "PC", 1)
    if flags["Z"]:
        assert isinstance(value, int)
        regs["PC"] = value


def run_prog(prog: list[int], sleep_time: int = 0):
    Globals = globals()

    mem = prog
    mem.extend([0 for x in range(256 - len(prog))])
    regs = {"PC": 0, "A": 0, "B": 0, "O": 0}
    flags = {"C": False, "Z": False}
    halt: Optional[bool] = None

    while halt is None:
        ins = mem[regs["PC"]]
        inc(regs, "PC", 1)
        halt = Globals["ins_" + str(ins)](regs, mem, flags)
        print(
            *[f"{k}: {v:>08b}" for k, v in regs.items()],
            f"O: {regs['O']}",
            f'A+B: {(regs["A"] + regs["B"]) & 255:>08b}',
            mem[24],
            sep="\t",
        )
        if sleep_time:
            sleep(sleep_time)


# fmt: off
add_NUM = 9
add: list[Union[str, int]] = [
    "OUTA",
    "ADDI", 1,
    "JMP", 0,
]

# fmt: off
fib_TMP = 21
fib_OLD = 22
fib: list[Union[str, int]] = [
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

# fmt: off
pat: list[Union[str, int]] = [
    "LDIA", 1,   "LDIB", 1,   "OUTI", 1,
    "LDIA", 2,   "LDIB", 2,   "OUTI", 2,
    "LDIA", 4,   "LDIB", 4,   "OUTI", 4,
    "LDIA", 8,   "LDIB", 8,   "OUTI", 8,
    "LDIA", 16,  "LDIB", 16,  "OUTI", 16,
    "LDIA", 32,  "LDIB", 32,  "OUTI", 32,
    "LDIA", 64,  "LDIB", 64,  "OUTI", 64,
    "LDIA", 128, "LDIB", 128, "OUTI", 128,
    "LDIA", 128, "LDIB", 128, "OUTI", 128,
    "LDIA", 64,  "LDIB", 64,  "OUTI", 64,
    "LDIA", 32,  "LDIB", 32,  "OUTI", 32,
    "LDIA", 16,  "LDIB", 16,  "OUTI", 16,
    "LDIA", 8,   "LDIB", 8,   "OUTI", 8,
    "LDIA", 4,   "LDIB", 4,   "OUTI", 4,
    "LDIA", 2,   "LDIB", 2,   "OUTI", 2,
    "LDIA", 1,   "LDIB", 1,   "OUTI", 1,
    "JMP", 0,    
]


# for n+1 
  # check if n is prime
# fmt: off
prime = [
    Ins.LDA, 25,
    Ins.OUTA,
    "LDA", 24, # 3
    "ADD", 23,
    "STA", 24,
    "LDA", 25,
    "SUB", 24, # 11
    "JMPZ", 19,
        "JMPNC", 3,
            "JMP", 11,
    "LDA", 24, # 19
    "OUTA",
    "HLT",
    1, # 23
    1, # 24
    23, # 25
]


halt = False
run_prog(prime)
