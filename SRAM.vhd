library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.std_logic_arith.all;

entity sram_ctrl is
port(
	clk : in std_logic;
	rst_n : in std_logic;
	
	-- Avalon slave
	read : in std_logic;
	write : in std_logic;
	waitrequest : out std_logic;
	address : in std_logic_vector(17 downto 0);
	readdata : out std_logic_vector(15 downto 0);
	writedata : in std_logic_vector(15 downto 0);
	byteenable : in std_logic_vector(1 downto 0);
	
	-- SRAM
	sram_ce_n : out std_logic;
	sram_addr : out std_logic_vector(17 downto 0);
	sram_dq : inout std_logic_vector(15 downto 0);
	sram_oe_n : out std_logic;
	sram_we_n : out std_logic;
	sram_lb_n : out std_logic;
	sram_ub_n : out std_logic
);
end entity;

architecture rtl of sram_ctrl is

type t_state is (IDLE, READ1, READ2, WRITE1, WRITE2);
signal state_r : t_state;

signal activity : std_logic;

begin

XREG_ST : process(clk, rst_n)
begin
	if rst_n = '0' then
		readdata <= (others=>'0');
		state_r <= IDLE;
		sram_ce_n <= '1';
		sram_addr <= (others=>'0');
		sram_dq <= (others=>'Z');
		sram_oe_n <= '1';
		sram_we_n <= '1';
		sram_lb_n <= '1';
		sram_ub_n <= '1';
	elsif clk'event and clk = '1' then
		readdata <= sram_dq;
		sram_addr <= address;

		-- Default values
		sram_oe_n <= '1';
		sram_we_n <= '1';
		sram_ce_n <= '1';
		sram_dq <= (others=>'Z');
		sram_lb_n <= '1';
		sram_ub_n <= '1';
				
		case state_r is
		when IDLE =>
			if activity = '1' then 
				sram_ce_n <= '0';
				sram_lb_n <= not byteenable(0);
				sram_ub_n <= not byteenable(1);
			end if;
			
			if read = '1' then
				state_r <= READ1;
				sram_oe_n <= '0';
			elsif write = '1' then
				state_r <= WRITE1;
				sram_we_n <= '0';
				sram_dq <= writedata;
			end if;
		when READ1 =>
			sram_ce_n <= '0';
			sram_oe_n <= '0';
			sram_lb_n <= not byteenable(0);
			sram_ub_n <= not byteenable(1);
			state_r <= READ2;
		when READ2 =>
			state_r <= IDLE;
		when WRITE1 =>
			sram_ce_n <= '0';
			sram_we_n <= '0';
			sram_lb_n <= not byteenable(0);
			sram_ub_n <= not byteenable(1);
			sram_dq <= writedata;
			state_r <= WRITE2;
		when WRITE2 =>
			state_r <= IDLE;
		end case;
	end if;
end process;

activity <= read or write;
waitrequest <= '1' when ((state_r = IDLE) and activity = '1') or
								(state_r = READ1) or
								(state_r = WRITE1) 
								else '0';
								
end architecture;