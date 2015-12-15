library(ggplot2)
library(plyr)
library(lattice)

datafile = commandArgs(TRUE)[1]
wholeframe = read.table(datafile, header=TRUE)
framsum = ddply(wholeframe, c("Version","Threads", "Size"), summarize, MeanTime = mean(Time), SDev = sd(Time))

pd = position_dodge(width=.1)


baseplot = ggplot(framsum)
pdf(paste(datafile,".sequential.pdf", sep = ''), width = 10, height=6)
myplot = baseplot + geom_line(aes(x=Size, y=MeanTime, group=Version, color=Version))
myplot = myplot + geom_errorbar(aes(x=Size, ymin=MeanTime-SDev, ymax=MeanTime+SDev, width=.1))
#myplot = myplot + facet_grid(~Numactl)
myplot = myplot + guides(col = guide_legend(ncol=3))
myplot = myplot + theme(legend.text = element_text(size=8), legend.title = element_text(size=8), legend.position="bottom")
myplot = myplot + ggtitle("Performance evaluation for various size on 1 threads")
myplot = myplot + ylab("Time (s)")
myplot = myplot + xlab("# of elements in the list")
print(myplot)
dev.off()


baseplot = ggplot(subset(framsum, Version=="Parallel" & Size > 1000000))
pdf(paste(datafile,".parallel.pdf", sep = ''), width = 10, height=6)
myplot = baseplot + geom_line(aes(x=Threads, y=MeanTime, group=factor(Size), color=factor(Size)))
myplot = myplot + geom_errorbar(aes(x=Threads, ymin=MeanTime-SDev, ymax=MeanTime+SDev, width=.1))
#myplot = myplot + facet_grid(~Numactl)
myplot = myplot + guides(col = guide_legend(ncol=3))
myplot = myplot + theme(legend.text = element_text(size=8), legend.title = element_text(size=8), legend.position="bottom")
myplot = myplot + ggtitle("Performance evaluation for parallel version on various sizes")
myplot = myplot + ylab("Time (s)")
myplot = myplot + xlab("# of elements in the list")
print(myplot)
dev.off()


