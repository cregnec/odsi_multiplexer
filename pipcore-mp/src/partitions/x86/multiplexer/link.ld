/*******************************************************************************/
/*  © Université Lille 1, The Pip Development Team (2015-2016)                 */
/*                                                                             */
/*  This software is a computer program whose purpose is to run a minimal,     */
/*  hypervisor relying on proven properties such as memory isolation.          */
/*                                                                             */
/*  This software is governed by the CeCILL license under French law and       */
/*  abiding by the rules of distribution of free software.  You can  use,      */
/*  modify and/ or redistribute the software under the terms of the CeCILL     */
/*  license as circulated by CEA, CNRS and INRIA at the following URL          */
/*  "http://www.cecill.info".                                                  */
/*                                                                             */
/*  As a counterpart to the access to the sender code and  rights to copy,     */
/*  modify and redistribute granted by the license, users are provided only    */
/*  with a limited warranty  and the software's author,  the holder of the     */
/*  economic rights,  and the successive licensors  have only  limited         */
/*  liability.                                                                 */
/*                                                                             */
/*  In this respect, the user's attention is drawn to the risks associated     */
/*  with loading,  using,  modifying and/or developing or reproducing the      */
/*  software by the user in light of its specific status of free software,     */
/*  that may mean  that it is complicated to manipulate,  and  that  also      */
/*  therefore means  that it is reserved for developers  and  experienced      */
/*  professionals having in-depth computer knowledge. Users are therefore      */
/*  encouraged to load and test the software's suitability as regards their    */
/*  requirements in conditions enabling the security of their systems and/or   */
/*  data to be ensured and,  more generally, to use and operate it in the      */
/*  same conditions as regards security.                                       */
/*                                                                             */
/*  The fact that you are presently reading this means that you have had       */
/*  knowledge of the CeCILL license and that you accept its terms.             */
/*******************************************************************************/

OUTPUT_FORMAT("binary")
ENTRY(_start)

physbase = 0x00700000;

SECTIONS
{
  .text 0x700000 :
  {
    code = .; _code = .; __code = .;
    *(.text.entry)
    *(.text)
    . = ALIGN(4096);
  }

	. = ALIGN(4096);
  .bss :
  {
    bss = .; _bss = .; __bss = .;
    *(.bss)
    . = ALIGN(4096);
  }

  .data :
  {
     data = .; _data = .; __data = .;
     *(.data)
     *(.rodata)
     . = ALIGN(4096);
  }

  .partition_freertos : ALIGN(4096)
  {
      _partition_freertos = . ;
      *(.freertos)
      . = ALIGN(0x40000);
      _epartition_freertos = . ;
  }

    .partition_secure : ALIGN(4096)
  {
      _partition_secure = . ;
      *(.secure)
      . = ALIGN(0x40000);
      _epartition_secure = . ;
  }

    .partition_normal : ALIGN(4096)
  {
      _partition_normal = . ;
      *(.normal)
      . = ALIGN(0x40000);
      _epartition_normal = . ;
  }
}

