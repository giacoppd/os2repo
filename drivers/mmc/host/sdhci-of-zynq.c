/*
 * Copyright (c) 2015 Wind River Systems, Inc.
 *
 * Xilinx Zynq Secure Digital Host Controller Interface.
 * Based on sdhci-of-arasan.c
 *
 * Copyright (c) 2007 Freescale Semiconductor, Inc.
 * Copyright (c) 2009 MontaVista Software, Inc.
 *
 * Authors: Xiaobo Xie <X.Xie@freescale.com>
 *	    Anton Vorontsov <avorontsov@ru.mvista.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 */

#include <linux/module.h>
#include "sdhci-pltfm.h"

#define SDHCI_zynq_CLK_CTRL_OFFSET	0x2c

#define CLK_CTRL_TIMEOUT_SHIFT		16
#define CLK_CTRL_TIMEOUT_MASK		(0xf << CLK_CTRL_TIMEOUT_SHIFT)
#define CLK_CTRL_TIMEOUT_MIN_EXP	13

/**
 * struct sdhci_zynq_data
 * @aper_clk:	Pointer to the AHB clock
 */
struct sdhci_zynq_data {
	struct clk	*aper_clk;
};

static unsigned int sdhci_zynq_get_timeout_clock(struct sdhci_host *host)
{
	u32 div;
	unsigned long freq;
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);

	div = readl(host->ioaddr + SDHCI_zynq_CLK_CTRL_OFFSET);
	div = (div & CLK_CTRL_TIMEOUT_MASK) >> CLK_CTRL_TIMEOUT_SHIFT;

	freq = clk_get_rate(pltfm_host->clk);
	freq /= 1 << (CLK_CTRL_TIMEOUT_MIN_EXP + div);

	return freq;
}

static struct sdhci_ops sdhci_zynq_ops = {
	.get_max_clock = sdhci_pltfm_clk_get_max_clock,
	.get_timeout_clock = sdhci_zynq_get_timeout_clock,
};

static struct sdhci_pltfm_data sdhci_zynq_pdata = {
	.ops = &sdhci_zynq_ops,
};

#ifdef CONFIG_PM_SLEEP
/**
 * sdhci_zynq_suspend - Suspend method for the driver
 * @dev:	Address of the device structure
 * Returns 0 on success and error value on error
 *
 * Put the device in a low power state.
 */
static int sdhci_zynq_suspend(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct sdhci_host *host = platform_get_drvdata(pdev);
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_zynq_data *sdhci_zynq = pltfm_host->priv;
	int ret;

	ret = sdhci_suspend_host(host);
	if (ret)
		return ret;

	clk_disable(pltfm_host->clk);
	clk_disable(sdhci_zynq->aper_clk);

	return 0;
}

/**
 * sdhci_zynq_resume - Resume method for the driver
 * @dev:	Address of the device structure
 * Returns 0 on success and error value on error
 *
 * Resume operation after suspend
 */
static int sdhci_zynq_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct sdhci_host *host = platform_get_drvdata(pdev);
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_zynq_data *sdhci_zynq = pltfm_host->priv;
	int ret;

	ret = clk_enable(sdhci_zynq->aper_clk);
	if (ret) {
		dev_err(dev, "Cannot enable AHB clock.\n");
		return ret;
	}

	ret = clk_enable(pltfm_host->clk);
	if (ret) {
		dev_err(dev, "Cannot enable SD clock.\n");
		clk_disable(sdhci_zynq->aper_clk);
		return ret;
	}

	return sdhci_resume_host(host);
}
#endif /* ! CONFIG_PM_SLEEP */

static SIMPLE_DEV_PM_OPS(sdhci_zynq_dev_pm_ops, sdhci_zynq_suspend,
			 sdhci_zynq_resume);

static int sdhci_zynq_probe(struct platform_device *pdev)
{
	int ret;
	struct clk *ref_clk;
	struct sdhci_host *host;
	struct sdhci_pltfm_host *pltfm_host;
	struct sdhci_zynq_data *sdhci_zynq;

	sdhci_zynq = devm_kzalloc(&pdev->dev, sizeof(*sdhci_zynq),
			GFP_KERNEL);
	if (!sdhci_zynq)
		return -ENOMEM;

	sdhci_zynq->aper_clk = devm_clk_get(&pdev->dev, "aper_clk");
	if (IS_ERR(sdhci_zynq->aper_clk)) {
		dev_err(&pdev->dev, "aper_clk clock not found.\n");
		return PTR_ERR(sdhci_zynq->aper_clk);
	}

	ref_clk = devm_clk_get(&pdev->dev, "ref_clk");
	if (IS_ERR(ref_clk)) {
		dev_err(&pdev->dev, "ref_clk clock not found.\n");
		return PTR_ERR(ref_clk);
	}

	ret = clk_prepare_enable(sdhci_zynq->aper_clk);
	if (ret) {
		dev_err(&pdev->dev, "Unable to enable AHB clock.\n");
		return ret;
	}

	ret = clk_prepare_enable(ref_clk);
	if (ret) {
		dev_err(&pdev->dev, "Unable to enable SD clock.\n");
		goto clk_dis_ahb;
	}

	host = sdhci_pltfm_init(pdev, &sdhci_zynq_pdata, 0);
	if (IS_ERR(host)) {
		ret = PTR_ERR(host);
		dev_err(&pdev->dev, "platform init failed (%u)\n", ret);
		goto clk_disable_all;
	}

	sdhci_get_of_property(pdev);
	pltfm_host = sdhci_priv(host);
	pltfm_host->priv = sdhci_zynq;
	pltfm_host->clk = ref_clk;

	ret = sdhci_add_host(host);
	if (ret) {
		dev_err(&pdev->dev, "platform register failed (%u)\n", ret);
		goto err_pltfm_free;
	}

	return 0;

err_pltfm_free:
	sdhci_pltfm_free(pdev);
clk_disable_all:
	clk_disable_unprepare(ref_clk);
clk_dis_ahb:
	clk_disable_unprepare(sdhci_zynq->aper_clk);

	return ret;
}

static int sdhci_zynq_remove(struct platform_device *pdev)
{
	struct sdhci_host *host = platform_get_drvdata(pdev);
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_zynq_data *sdhci_zynq = pltfm_host->priv;

	clk_disable_unprepare(pltfm_host->clk);
	clk_disable_unprepare(sdhci_zynq->aper_clk);

	return sdhci_pltfm_unregister(pdev);
}

static const struct of_device_id sdhci_zynq_of_match[] = {
	{ .compatible = "xlnx,ps7-sdio-1.00.a"},
	{ .compatible = "arasan,sdhci-8.9a"},
	{ .compatible = "generic-sdhci"},
	{ }
};
MODULE_DEVICE_TABLE(of, sdhci_zynq_of_match);

static struct platform_driver sdhci_zynq_driver = {
	.driver = {
		.name = "sdhci-zynq",
		.owner = THIS_MODULE,
		.of_match_table = sdhci_zynq_of_match,
		.pm = &sdhci_zynq_dev_pm_ops,
	},
	.probe = sdhci_zynq_probe,
	.remove = sdhci_zynq_remove,
};

module_platform_driver(sdhci_zynq_driver);

MODULE_DESCRIPTION("Driver for the Zynq SDHCI Controller");
MODULE_AUTHOR("Soeren Brinkmann <soren.brinkmann@xilinx.com>");
MODULE_LICENSE("GPL");
