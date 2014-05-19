/*
* Simd Library Tests.
*
* Copyright (c) 2011-2014 Yermalayeu Ihar.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy 
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
* copies of the Software, and to permit persons to whom the Software is 
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in 
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/
#include "Test/TestUtils.h"
#include "Test/TestPerformance.h"
#include "Test/Test.h"

namespace Test
{
	namespace
	{
		struct FuncSR
		{
			typedef void(*FuncPtr)(const uint8_t * mask, size_t stride, size_t width, size_t height, uint8_t index,
                ptrdiff_t * left, ptrdiff_t * top, ptrdiff_t * right, ptrdiff_t * bottom);
			FuncPtr func;
			std::string description;

			FuncSR(const FuncPtr & f, const std::string & d) : func(f), description(d) {}

			void Call(const View & src, uint8_t index, const Rect & srcRect, Rect & dstRect) const
			{
                dstRect = srcRect;
				TEST_PERFORMANCE_TEST(description);
				func(src.data, src.stride, src.width, src.height, index, &dstRect.left, &dstRect.top, &dstRect.right, &dstRect.bottom);
			}
		};	
	}

#define FUNC_SR(func) FuncSR(func, #func)

    bool SegmentationShrinkRegionTest(int width, int height, const FuncSR & f1, const FuncSR & f2)
    {
        bool result = true;

        std::cout << "Test " << f1.description << " & " << f2.description << " for size [" << width << "," << height << "]." << std::endl;

        const uint8_t index = 3;
        View s(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        Rect rs1(s.Size()), rs2(s.Size()), rd1, rd2;
        Simd::Fill(s, 0);
        FillRandomMask(s.Region(width*3/16, height*5/16, width*11/16, height*13/16).Ref(), index);

        TEST_EXECUTE_AT_LEAST_MIN_TIME(f1.Call(s, index, rs1, rd1));

        TEST_EXECUTE_AT_LEAST_MIN_TIME(f2.Call(s, index, rs2, rd2));

        result = result && Compare(rd1, rd2, true);

        return result;
    }

    bool SegmentationShrinkRegionTest()
    {
        bool result = true;

        result = result && SegmentationShrinkRegionTest(W, H, FUNC_SR(Simd::Base::SegmentationShrinkRegion), FUNC_SR(SimdSegmentationShrinkRegion));
        result = result && SegmentationShrinkRegionTest(W + 1, H - 1, FUNC_SR(Simd::Base::SegmentationShrinkRegion), FUNC_SR(SimdSegmentationShrinkRegion));
        result = result && SegmentationShrinkRegionTest(W - 1, H + 1, FUNC_SR(Simd::Base::SegmentationShrinkRegion), FUNC_SR(SimdSegmentationShrinkRegion));

#if defined(SIMD_SSE41_ENABLE) && defined(SIMD_AVX2_ENABLE)
        if(Simd::Sse41::Enable && Simd::Avx2::Enable)
        {
            result = result && SegmentationShrinkRegionTest(W, H, FUNC_SR(Simd::Sse41::SegmentationShrinkRegion), FUNC_SR(Simd::Avx2::SegmentationShrinkRegion));
            result = result && SegmentationShrinkRegionTest(W + 1, H - 1, FUNC_SR(Simd::Sse41::SegmentationShrinkRegion), FUNC_SR(Simd::Avx2::SegmentationShrinkRegion));
            result = result && SegmentationShrinkRegionTest(W - 1, H + 1, FUNC_SR(Simd::Sse41::SegmentationShrinkRegion), FUNC_SR(Simd::Avx2::SegmentationShrinkRegion));
        }
#endif

        return result;    
    }
}