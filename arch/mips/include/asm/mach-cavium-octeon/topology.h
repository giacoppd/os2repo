#ifndef _ASM_MACH_CAVIUM_OCTEON_TOPOLOGY_H
#define _ASM_MACH_CAVIUM_OCTEON_TOPOLOGY_H

#ifdef CONFIG_NUMA

static inline int cpu_to_node(int cpu)
{
	return (cpu_logical_map(cpu) >> 7) & 7;
}

static inline struct cpumask *cpumask_of_node(int node)
{
	if (node == -1)
		return cpu_all_mask;
	return &__node_data[node].cpumask_on_node;
}

static inline int parent_node(int node)
{
	return node;
}

struct pci_bus;
static inline int pcibus_to_node(struct pci_bus *bus)
{
	return 0;
}

static inline struct cpumask *cpumask_of_pcibus(struct pci_bus *bus)
{
	return cpumask_of_node(pcibus_to_node(bus));
}
#endif /* CONFIG_NUMA */

static inline int pa_to_nid(u64 pa)
{
	return (pa >> 40) & 7;
}

#include <asm-generic/topology.h>

#endif /* _ASM_MACH_CAVIUM_OCTEON_TOPOLOGY_H */
