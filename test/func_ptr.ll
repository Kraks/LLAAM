; ModuleID = 'func_ptr.c'
source_filename = "func_ptr.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

; Function Attrs: nounwind ssp uwtable
define i32 @add(i32, i32) #0 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  store i32 %0, i32* %3, align 4
  store i32 %1, i32* %4, align 4
  %5 = load i32, i32* %3, align 4
  %6 = load i32, i32* %4, align 4
  %7 = add nsw i32 %5, %6
  ret i32 %7
}

; Function Attrs: nounwind ssp uwtable
define i32 @main() #0 {
  %1 = alloca i32 (i32, i32)*, align 8
  %2 = alloca i32 (i32, i32)*, align 8
  %3 = alloca i32, align 4
  store i32 (i32, i32)* @add, i32 (i32, i32)** %1, align 8
  %4 = load i32 (i32, i32)*, i32 (i32, i32)** %1, align 8
  store i32 (i32, i32)* %4, i32 (i32, i32)** %2, align 8
  %5 = load i32 (i32, i32)*, i32 (i32, i32)** %2, align 8
  %6 = call i32 %5(i32 3, i32 4)
  store i32 %6, i32* %3, align 4
  ret i32 0
}

attributes #0 = { nounwind ssp uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"PIC Level", i32 2}
!1 = !{!"Apple LLVM version 8.0.0 (clang-800.0.42.1)"}
