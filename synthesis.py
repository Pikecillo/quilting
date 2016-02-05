import cv2
import numpy
import random

"""
Pick a random patch from a texture
"""
def random_patch(in_tex, patch_size):
    (in_height, in_width) = in_tex.shape[:-1]
    (p_height, p_width) = patch_size

    ri = random.uniform(0, in_height - p_height + 1)
    rj = random.uniform(0, in_width - p_width + 1)

    return in_tex[ri : ri + p_height, rj : rj + p_width]

"""
Random tiling: the most naive approach. Tile by picking random patches
"""
def random_tiling(in_tex, out_size, block_size):
    (in_height, in_width, channels) = in_tex.shape
    (b_height, b_width) = block_size
    (out_height, out_width) = out_size

    out_tex = numpy.zeros(dtype='uint8',
                          shape=(out_height, out_width, channels))

    for i in range(0, out_height, b_height):
        for j in range(0, out_width, b_width):
            iend = min(i + b_height, out_height - 1)
            jend = min(j + b_width, out_width - 1)

            patch_size = (min(b_height, iend - i), min(b_width, jend - j))
            out_tex[i : iend, j : jend] = random_patch(in_tex, patch_size)

    return out_tex

"""
Overlap constrained: a less naive approach. Tile by choosing patches
that best match the current output texture
"""
def overlap_constrained(in_tex, out_size, block_size, overlap):
    (in_height, in_width, channels) = in_tex.shape
    (b_height, b_width) = block_size
    (out_height, out_width) = out_size

    out_tex = numpy.ones(dtype='uint8',
                         shape=(out_size[0], out_size[1], channels)) * 128

    for i in range(0, out_height, b_height - overlap):
        for j in range(0, out_width, b_width - overlap):
            iend = min(i + b_height, out_height - 1)
            jend = min(j + b_width, out_width - 1)
            patch_size = (min(b_height, iend - i), min(b_width, jend - j))

            # The first patch is random
            if i == 0 and j == 0:
                out_tex[i : iend, j : jend] = random_patch(in_tex, patch_size)
            else:
                out_tex[i : iend, j : jend] = best_match(in_tex, out_tex,
                                                         (i, j), patch_size,
                                                         overlap)

    return out_tex

"""
Pick best match for current stage of synthesis
"""
def best_match(in_tex, out_tex, out_position, patch_size, overlap):
    (in_height, in_width, channels) = in_tex.shape
    (p_height, p_width) = patch_size
    (i_out, j_out) = out_position
    best = 10 ** 10
    best_patch = None

    out_vslice = numpy.float32(out_tex[i_out : i_out + overlap,
                                       j_out : j_out + p_width])
    out_hslice = numpy.float32(out_tex[i_out : i_out + p_height,
                                       j_out : j_out + overlap])

    for i in range(0, in_height - p_height):
        for j in range(0, in_width - p_width):
            v_error = h_error = 0
            
            if i_out != 0:
                in_slice = numpy.float32(in_tex[i : i + overlap,
                                                j : j + p_width])
                v_error = ((in_slice - out_vslice) ** 2).sum() / in_slice.size

            if j_out != 0:
                in_slice = numpy.float32(in_tex[i : i + p_height,
                                                j : j + overlap])
                v_error = ((in_slice - out_hslice) ** 2).sum() / in_slice.size

            if h_error + v_error < best:
                best = h_error + v_error
                best_patch = numpy.float32(in_tex[i : i + p_height,
                                                  j : j + p_width])

    return best_patch

if __name__ == '__main__':
    tex = cv2.imread('images/olives.jpg')

    random_tex = random_tiling(tex, (400, 400), (90, 90))
    constr_tex = overlap_constrained(tex, (400, 400), (90, 90), 20)

    cv2.imshow('Sample', tex)
    cv2.imshow('Random Tiling', random_tex)
    cv2.imshow('Overlap Constrained', constr_tex)

    cv2.waitKey(0)
    cv2.destroyAllWindows()
