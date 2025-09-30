all : flash

TARGET:=sine

TARGET_MCU?=CH32V003
include ./ch32fun/ch32fun.mk

flash : cv_flash
clean : cv_clean
