import random
import math


def swap(arr, ix1, ix2):
    t = arr[ix1]
    arr[ix1] = arr[ix2]
    arr[ix2] = t
    return arr


def max_heapify(arr, i, n):
    left_child_ix = 2 * i + 1
    right_child_ix = 2 * i + 2

    hi = i

    if (left_child_ix < n
            and arr[left_child_ix] > arr[hi]):
        hi = left_child_ix
    if (right_child_ix < n
            and arr[right_child_ix] > arr[hi]):
        hi = right_child_ix

    if (hi != i):
        swap(arr, i, hi)
        max_heapify(arr, hi, n)


def build_max_heap(arr):
    n = len(arr)
    m = int(math.floor(n / 2))
    for i in range(m - 1, -1, -1):
        max_heapify(arr, i, len(arr))


def heap_sort(arr):
    build_max_heap(arr)
    for i in range(len(arr)):
        n = len(arr) - (i + 1)
        swap(arr, 0, n)
        max_heapify(arr, 0, n)
    return arr


def insertion_sort(arr):
    for i in range(1, len(arr)):
        key = arr[i]
        pos = i

        while (pos > 0 and arr[pos - 1] > key):
            swap(arr, pos, pos - 1)
            pos -= 1

        arr[pos] = key


def partition(arr, lo, hi):
    pivot = arr[hi]

    i = lo

    for j in range(lo, hi):
        if (arr[j] < pivot):
            swap(arr, i, j)
            i += 1

    swap(arr, i, hi)
    return i


def _quick_sort_inplace(arr, lo, hi):
    if (lo < hi):
        pivot = partition(arr, lo, hi)
        _quick_sort_inplace(arr, lo, pivot - 1)
        _quick_sort_inplace(arr, pivot + 1, hi)


def quick_sort(arr):
    n = len(arr) - 1
    return _quick_sort_inplace(arr, 0, n)


arr = [5, 7, 2, 1, 99, 4, 2]
sarr = quick_sort(arr)
print(sarr)

# arr = [5, 2, 7, 9, 1]
# insertion_sort(arr)

# arr = [5, 12, 10, 7, 6, 8, 9]
# sarr = heap_sort(arr)
# print(sarr)
