# Oposition

## Overall
- Good, but needs restructuring to put more interesting stuff in the Introduction. I would put much of the stuff from Approach there because it just sets the scene for your contributions.
- A good thesis report. The results section is thorough and interesting to read. It gets better the closer you come to the end, so consider restructuring to put more appealing stuff earlier. The first few pages are weak and don't explain where the report is heading. I did not understand what the scope of the report was until half-way into the Approach chapter. It needs a better Introduction to encourage people to continue reading.

## Questions
- (H264, does Axis use H265 or is it too processor/memory heavy?)
- Splitting images horizontally, how does it affect compression efficiency, can the the processes access each other's pixels for predictions?
- What is the timing issue for horizontal splitting? The two processors should be kept in sync (easy enough, no?) and the process will always wait for the slowest processor, but are there other problems?
- How does vertical split differ from single-processor?
- What about measuring bus usage and memory traffic in the virtual environment, surely that would be better than just reasoning about it.
- I like the honesty of saying you cut corners, but why did you chose to do this? Time constraints, or were you not able to do it in the "correct", if so does this indicate that this would be equally hard to do in hardware?
- Why is the 4K stream only in 30 fps?
- You spend time talking about the splitting, then just chose the "vertical" one. Did you consider measuring performance for the other ones too (maybe to cumbersome to actually implement?), and if not why bring them up at all if they are not viable alternatives?

## Praise
- Easy to read, good language.
- Chapter 3 stands out in terms of quality. An honest analysis of the weaknesses, and then a thorough evaluation of each component and well reasoned.
- Good that you mention "360 degree panorama for the ARTPEC-5" and how it makes this method infeasible for 4K.
- I like MSI struct in 2.16 and appendices B and C, which give me an insight into the programming concepts.
- Figure 2.7 is beautiful in its clarity!
- 3.2.2 is good. Clearly states that dual chips is better because it is cheaper.

## Criticism
- Into seems severely lacking, no real introduction to area at all. I'm lost!
- Explain concepts/acronyms on first use: ARTPEC, PCIe, POSIX, memory mapped region, LCPU, TLMu and QEMU, I had to google to keep track on what was happening in the beginning of the report.
- "The interconnection went through a series of different implementations before we found the correct approach." How do we know this is the final, correct version?
- All the forward references in the early stages are a bit confusing. (See later section, etc.)
- I would move everything before 2.2.4  + 2.2.6 to introduction. And explain all the basics which are currently missing.
- "The calculations assume 4:2:0 sub-sampling, resulting in 12 bits per" Explain what subsampling is, and why (1 + 1/4 + 1/4) * 8 = 12
- Some references, like to GCC, seem pretty pointless.

## Details
- Single dash should be an n-dash, written as "--" in LaTeX
- \usepackage[hidelinks]{hyperref} hides colored borders around links
- I-frame stands for Intra. "Inter frame" is the opposite, meaning P and B-frames!
- X86 -> x86
- we’re -> we are
- Tables referred to as figures
