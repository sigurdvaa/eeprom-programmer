from time import sleep
from typing import Callable
from enum import IntEnum


Ins = IntEnum(
    "Ins",
    [
        "HLT",
        "LDA",
        "LDB",
        "LDIA",
        "LDIB",
        "STA",
        "STB",
        "ADD",
        "ADDI",
        "SUB",
        "SUBI",
        "OUTA",
        "OUTI",
        "JMP",
        "JMPC",
        "JMPNC",
        "JMPZ",
    ],
    start=0,
)


class Computer:
    flags: dict[str, bool]
    ins: list[Callable]
    mem: list[int]
    regs: dict[str, int]
    halt: bool

    def __init__(self, prog: list[int]):
        self.mem = prog
        self.mem.extend([0 for x in range(256 - len(prog))])
        self.flags = {"C": False, "Z": False}
        self.halt = False
        self.regs = {"PC": 0, "A": 0, "B": 0, "O": 0}
        self.ins = [
            self.ins_HLT,
            self.ins_LDA,
            self.ins_LDB,
            self.ins_LDIA,
            self.ins_LDIB,
            self.ins_STA,
            self.ins_STB,
            self.ins_ADD,
            self.ins_ADDI,
            self.ins_SUB,
            self.ins_SUBI,
            self.ins_OUTA,
            self.ins_OUTI,
            self.ins_JMP,
            self.ins_JMPC,
            self.ins_JMPNC,
            self.ins_JMPZ,
        ]
        assert len(Ins) == len(self.ins)

    def inc(self, reg: str, amount: int):
        """Ensure we only have 8 bit values"""
        value = self.regs[reg] + amount
        self.regs[reg] = value & 255

    def ins_HLT(self):
        self.halt = True

    def ins_LDA(self):
        addr = self.mem[self.regs["PC"]]
        value = self.mem[addr]
        self.inc("PC", 1)
        self.regs["A"] = value

    def ins_LDB(self):
        addr = self.mem[self.regs["PC"]]
        value = self.mem[addr]
        self.inc("PC", 1)
        self.regs["B"] = value

    def ins_LDIA(self):
        value = self.mem[self.regs["PC"]]
        self.inc("PC", 1)
        self.regs["A"] = value

    def ins_LDIB(self):
        value = self.mem[self.regs["PC"]]
        self.inc("PC", 1)
        self.regs["B"] = value

    def ins_STA(self):
        addr = self.mem[self.regs["PC"]]
        self.inc("PC", 1)
        self.mem[addr] = self.regs["A"]

    def ins_STB(self):
        addr = self.mem[self.regs["PC"]]
        self.inc("PC", 1)
        self.mem[addr] = self.regs["B"]

    def ins_ADD(self):
        addr = self.mem[self.regs["PC"]]
        self.inc("PC", 1)
        value = self.mem[addr]
        self.regs["B"] = value
        self.flags["C"] = self.regs["A"] + value > 255
        self.inc("A", value)
        self.flags["Z"] = self.regs["A"] == 0

    def ins_ADDI(self):
        value = self.mem[self.regs["PC"]]
        self.regs["B"] = value
        self.inc("PC", 1)
        self.flags["C"] = self.regs["A"] + value > 255
        self.inc("A", value)
        self.flags["Z"] = self.regs["A"] == 0

    def ins_SUB(self):
        addr = self.mem[self.regs["PC"]]
        self.inc("PC", 1)
        value = self.mem[addr]
        self.regs["B"] = value
        self.flags["C"] = self.regs["A"] - value > -1
        self.inc("A", -value)
        self.flags["Z"] = self.regs["A"] == 0

    def ins_SUBI(self):
        value = self.mem[self.regs["PC"]]
        self.regs["B"] = value
        self.inc("PC", 1)
        self.flags["C"] = self.regs["A"] - value > -1
        self.inc("A", -value)
        self.flags["Z"] = self.regs["A"] == 0

    def ins_OUTA(self):
        self.regs["O"] = self.regs["A"]

    def ins_OUTI(self):
        value = self.mem[self.regs["PC"]]
        self.inc("PC", 1)
        self.regs["O"] = value

    def ins_JMP(self):
        value = self.mem[self.regs["PC"]]
        self.regs["PC"] = value

    def ins_JMPC(self):
        value = self.mem[self.regs["PC"]]
        self.inc("PC", 1)
        if self.flags["C"]:
            self.regs["PC"] = value

    def ins_JMPNC(self):
        value = self.mem[self.regs["PC"]]
        self.inc("PC", 1)
        old = self.regs["PC"]
        self.regs["PC"] = value
        if self.flags["C"]:
            self.regs["PC"] = old

    def ins_JMPZ(self):
        value = self.mem[self.regs["PC"]]
        self.inc("PC", 1)
        if self.flags["Z"]:
            self.regs["PC"] = value

    def run(self, sleep_time: float = 0.0):
        while not self.halt:
            curr_ins = self.mem[self.regs["PC"]]
            self.inc("PC", 1)
            self.ins[curr_ins]()
            print(
                *[f"{k}: {v:>08b}" for k, v in self.regs.items()],
                f"O: {self.regs['O']}",
                f'A+B: {(self.regs["A"] + self.regs["B"]) & 255:>08b}',
                self.mem[24],
                sep="\t",
            )
            if sleep_time:
                sleep(sleep_time)


# fmt: off
add = [
    Ins.OUTA,
    Ins.ADDI, 1,
    Ins.JMP, 0,
]

fib_TMP = 21
fib_OLD = 22
fibonacci = [
    Ins.OUTA,
    Ins.STA, fib_TMP,
    Ins.ADD, fib_OLD,
    Ins.LDB, fib_TMP,
    Ins.STB, fib_OLD,
    Ins.JMPC, 13,
        Ins.JMP, 0,
    Ins.OUTI, 0,
    Ins.LDIA, 1,
    Ins.STA, fib_OLD,
    Ins.JMP, 0,
    0, # fib_TMP
    1, # fib_OLD
]

pattern = [
    Ins.LDIA, 1,   Ins.LDIB, 1,   Ins.OUTI, 1,
    Ins.LDIA, 2,   Ins.LDIB, 2,   Ins.OUTI, 2,
    Ins.LDIA, 4,   Ins.LDIB, 4,   Ins.OUTI, 4,
    Ins.LDIA, 8,   Ins.LDIB, 8,   Ins.OUTI, 8,
    Ins.LDIA, 16,  Ins.LDIB, 16,  Ins.OUTI, 16,
    Ins.LDIA, 32,  Ins.LDIB, 32,  Ins.OUTI, 32,
    Ins.LDIA, 64,  Ins.LDIB, 64,  Ins.OUTI, 64,
    Ins.LDIA, 128, Ins.LDIB, 128, Ins.OUTI, 128,
    Ins.LDIA, 128, Ins.LDIB, 128, Ins.OUTI, 128,
    Ins.LDIA, 64,  Ins.LDIB, 64,  Ins.OUTI, 64,
    Ins.LDIA, 32,  Ins.LDIB, 32,  Ins.OUTI, 32,
    Ins.LDIA, 16,  Ins.LDIB, 16,  Ins.OUTI, 16,
    Ins.LDIA, 8,   Ins.LDIB, 8,   Ins.OUTI, 8,
    Ins.LDIA, 4,   Ins.LDIB, 4,   Ins.OUTI, 4,
    Ins.LDIA, 2,   Ins.LDIB, 2,   Ins.OUTI, 2,
    Ins.LDIA, 1,   Ins.LDIB, 1,   Ins.OUTI, 1,
    Ins.JMP, 0,    
]

# for n+1 
  # check if n is prime
prime = [
    Ins.LDA, 25,
    Ins.OUTA,
    Ins.LDA, 24, # 3
    Ins.ADD, 23,
    Ins.STA, 24,
    Ins.LDA, 25,
    Ins.SUB, 24, # 11
    Ins.JMPZ, 19,
        Ins.JMPNC, 3,
            Ins.JMP, 11,
    Ins.LDA, 24, # 19
    Ins.OUTA,
    Ins.HLT,
    1, # 23
    1, # 24
    23, # 25
]
# fmt: on

computer = Computer(fibonacci)
computer.run(0.01)
