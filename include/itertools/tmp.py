from typing import List
import numpy as np


def get_ndim(arr):

    def recurse(sub_arr, depth=0):
        max_depth = depth

        for i in sub_arr:
            if (isinstance(i, list)):
                max_depth = max(max_depth, recurse(i, depth + 1))
        return max_depth

    return recurse(arr)


def to_string(arr):
    delim = ", "
    ndim = get_ndim(arr)

    def recurse(sub_arr, ix=0):
        size = len(sub_arr)
        buff = ""

        spaces = " " * (ix + 1) if ix <= ndim else ""
        new_lines = "\n" * max(0, (ndim - ix))
        hanging_indent = new_lines + spaces

        for n, i in enumerate(sub_arr):
            if (isinstance(i, (list, tuple))):

                if (isinstance(i, list)):
                    t_buff = recurse(i, ix + 1)
                    t_buff = f"[{t_buff}]"
                else:
                    t_buff = to_string(i)
                    t_buff = f"({t_buff})"

                buff += hanging_indent + t_buff if n > 0 else t_buff
            else:
                buff += f"{i}"

            if (n < size - 1):
                buff += delim

        return buff

    s = recurse(arr, 0)
    return s


# arr = [[[1, 2, 3], [(7, [[8, 9, 10], [11, 12, 13]], 9), 5, 6]],
#        [[7, 8, 9], [10, 11, 12]],
#        [[13, 14, 15], [16, 17, 18]],
#        [[19, 20, 21], [22, 23, 24]]]

# s = to_string(arr)
# print(s)

# print(np.asarray(arr))


class iterator:
    def __init__(self, span: List, is_reversed = False) -> None:
        self.span = span
        self.begin = span[0][0]
        self.end = span[0][-1]
        self.is_reversed = is_reversed

    def get_value(self):
        return self.span[0] if not self.is_reversed else self.span[-1]


    def plus(self):
        next(self.get_value())
    
    def minus(self):
        pass

    def eq(self):
        pass
        



        

v1 = [0, 1, 2, 3]
v2 = [4, 5, 6, 7]
v3 = [8, 9, 10, 11]

iters = [v1, v2, v3]


