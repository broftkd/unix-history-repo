(File fixnum.l)
(cm-=& lambda quote list caddr >& > cdr <& < cadr fixp and or cond If)
(cm-= lambda quote list caddr cadr fixp or *throw throw |1+| setq terpri niceprint patom comp-msg progn comp-err cdr length eq =& = not cond If)
(d-fixop lambda d-clearreg d-fixnumbox e-write5 e-write4 e-write3 d-exp let e-cvt list d-simple quote concat fixp caddr cadr setq cdr d-callbig length eq not cond If prog)
(|c-\\| lambda quote d-fixop)
(cc->& lambda cc-eq caddr cadr list quote let)
(cm-> lambda quote list caddr cadr fixp or *throw throw |1+| setq terpri niceprint patom comp-msg progn comp-err cdr length eq =& = not cond If)
(cc-<& lambda cc-eq caddr cadr list quote let)
(cm-< lambda quote list caddr cadr fixp or *throw throw |1+| setq terpri niceprint patom comp-msg progn comp-err cdr length eq =& = not cond If)
(cc-oneminus lambda e-write4 and d-move eq not e-label e-write2 e-cvt e-write3 setq d-exp quote d-genlab cadr d-simple let e-goto car null cond If)
(cc-oneplus lambda e-write4 e-label and d-move eq not e-write2 e-cvt e-write3 setq d-exp quote d-genlab cadr d-simple let e-goto car null cond If)
(d-shiftcheck lambda cadr assoc cdr car dtpr eq and)
(d-fixnumcode lambda e-write4 d-shiftcheck e-cvt e-write3 d-fixexpand d-fixnumcode d-move eq not d-simple fixp setq cdr do list d-exp null cond If quote get car symbolp dtpr and let)
(d-collapse lambda quote eq or apply nreverse cdr *throw throw |1+| terpri niceprint patom comp-msg progn comp-err cons setq fixp car numberp cond If null do let)
(d-toplevmacroexpand lambda apply d-toplevmacroexpand getdisc eq bcdp or cond If cxr getd car symbolp dtpr and let)
(d-fixexpand lambda *throw throw |1+| terpri niceprint patom comp-msg progn comp-err list go return caar eq do assq null cddr cadr cdr d-collapse cons memq quote get car symbolp dtpr and cond If d-macroexpand setq prog)
(d-fixnumbox lambda d-clearreg e-writel d-genlab setq e-write2 e-write4 e-write3 let)
(c-fixnumop lambda d-fixnumbox d-fixnumexp)
(d-fixnumexp lambda d-fixexpand d-fixnumcode)
