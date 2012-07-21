library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity nespad_ctrl is
port(
	clk : in std_logic;
	rst_n : in std_logic;
	pad : out std_logic_vector(7 downto 0);
	pad_data : in std_logic;
	pad_latch : out std_logic;
	pad_clock : out std_logic
	);
end entity;

architecture rtl of nespad_ctrl is

signal clock_divider : std_logic_vector(23 downto 0);
signal clk_io : std_logic;
signal pad_r : std_logic_vector(7 downto 0);
signal pad_new : std_logic_vector(7 downto 0);
signal pad_st : std_logic_vector(4 downto 0);
signal data_r : std_logic;
signal pad_latch_next : std_logic;
signal pad_clock_next : std_logic;

begin

XREG : process(clk, rst_n)
begin
	if rst_n = '0' then
		clock_divider <= (others=>'0');
		pad_r <= (others=>'0');
		pad_latch <= '0';
		pad_clock <= '0';
	elsif clk'event and clk='1' then
		clock_divider <= clock_divider + 1;
		pad_r <= pad_new;
		pad_latch <= pad_latch_next;
		pad_clock <= pad_clock_next;
	end if;
end process;

XREG_PAD : process(clk_io, rst_n)
begin
	if rst_n = '0' then
		pad_st <= (others=>'0');
	elsif clk_io'event and clk_io='1' then
		data_r <= pad_data;
	    if(pad_st = "10001") then
			pad_st <= (others=>'0');
		else
			pad_st <= pad_st + 1;
		end if;
	end if;
end process;

pad_latch_next <= '1' when pad_st = "00001" else '0';
pad_clock_next <= '0' when pad_st = "00001" else pad_st(0);

XPADNEW : process(data_r, pad_st, pad_r)
begin
	pad_new <= pad_r;
	case pad_st is
	when "00011" => pad_new(0) <= data_r;
	when "00101" => pad_new(1) <= data_r;
	when "00111" => pad_new(2) <= data_r;
	when "01001" => pad_new(3) <= data_r;
	when "01011" => pad_new(4) <= data_r;
	when "01101" => pad_new(5) <= data_r;
	when "01111" => pad_new(6) <= data_r;
	when "10001" => pad_new(7) <= data_r;	
	when others =>
	end case;
end process;

clk_io <= clock_divider(15);
pad <= pad_r;



end architecture;